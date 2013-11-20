#include "modelviewer.h"

#include <QFile>
#include <QMouseEvent>
#include <QApplication>
#include <QMessageBox>
#include <qmath.h>

#include <iostream>

//----------------------------------------------------------------------------------------

static void qreal2glfloat(const QMatrix4x4 &in, GLfloat *out) {
#if QT_VERSION >= 0x050000
    const float *data = in.constData();
#else
    const qreal *data = in.constData();
#endif
    for (int i = 0; i < 16; ++i) out[i] = data[i];
}

// We have to repack matrices from qreal to GLfloat.
#define setUniformMatrix(func,location,value,cols,rows) \
        { \
        GLfloat mat[cols * rows]; \
        const qreal *data = value.constData(); \
        for (int i = 0; i < cols * rows; ++i) mat[i] = data[i]; \
        func(location, 1, GL_FALSE, mat); \
        }

ModelViewer::ModelViewer(const QGLFormat &fmt, QWidget *parent) : QGLWidget(new QGLContext(fmt), parent), model(0) {
    hAngle = 0;
    vAngle = 0;
    fovVal = 45.0;
    zPos = 15;
    pNear = 0.1;
    pFar = 100.0;
    uvMul = 1.0;
    minFiltering = GL_NEAREST;
    magFiltering = GL_NEAREST;
    outlineColor = QVector3D(0, 0, 0);
    drawOutline = true;
    drawMipLevels = false;
    drawRealMipmap = false;
}

ModelViewer::~ModelViewer() {
    model = 0;
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &uvBuffer);
    glDeleteTextures(1, &textureID);
    glDeleteTextures(1, &mipmapTextureID);
    glDeleteProgram(shaderProgramID);
    glDeleteVertexArrays(1, &vertexArrayID);
}

void ModelViewer::setModel(OBJModel *m) {
    if(model) {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &uvBuffer);
        glDeleteTextures(1, &textureID);
    }

    std::vector<OBJVec3> vs, ns;
    std::vector<OBJVec2> ts;
    for(std::vector<OBJFace>::iterator fi = m->faces.begin(); fi != m->faces.end(); ++fi) {
        for(OBJFace::iterator fii = fi->begin(); fii != fi->end(); ++fii) {
            if(fii->n != 0) ns.push_back(m->norms[fii->n - 1]);
            if(fii->t != 0) ts.push_back(OBJVec2(m->texs[fii->t - 1].x, m->texs[fii->t - 1].y));
            vs.push_back(m->verts[fii->v - 1]);
        }
    }

    //assume there is only triangles in the model
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(OBJVec3), &vs[0], GL_STATIC_DRAW);
    vertexBufferSize = vs.size();

    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, ts.size() * sizeof(OBJVec2), &ts[0], GL_STATIC_DRAW);

    //assume that the model always has a texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m->texture.width(), m->texture.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, m->texture.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFiltering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiltering);

    generateRealMipmap(m->texture.width(), m->texture.height());

    model = m;
    resetView();
    update();
}

void ModelViewer::generateRealMipmap(int w, int h) {
    if(model) {
        glDeleteTextures(1, &mipmapTextureID);
    }
    glGenTextures(1, &mipmapTextureID);
    glBindTexture(GL_TEXTURE_2D, mipmapTextureID);
    int i = 0;
    float maxLevels = log2(qMax(w, h));
    int step = 255 / maxLevels;
    for(int szw = w, szh = h; i <= maxLevels; ++i, szw = qMax(szw / 2, 1), szh = qMax(szh / 2, 1)) {
        QImage img(szw, szh, QImage::Format_RGB888);
        int c = qMin(255, step * i);
        img.fill(QColor(c, c, c));
        glTexImage2D(GL_TEXTURE_2D, i, GL_RGB, szw, szh, 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFiltering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiltering);
}

void ModelViewer::setMeshColor(QVector3D mc) {
    outlineColor = mc;
    update();
}

void ModelViewer::setTexCoordsMultiplier(double val) {
    uvMul = val;
    update();
}

void ModelViewer::setFilteringType(int ft) {
    switch(ft) {
    case GL_NEAREST: case GL_LINEAR:
        magFiltering = ft;
        break;
    default:
        magFiltering = GL_LINEAR;
    }
    minFiltering = ft;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFiltering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiltering);
    update();
}

void ModelViewer::setDrawOutline(bool val) {
    drawOutline = val;
    update();
}

void ModelViewer::setDrawMipLevels(bool val) {
    drawMipLevels = val;
    update();
}

void ModelViewer::setDrawRealMipmap(bool val) {
    drawRealMipmap = val;
    if(val) glBindTexture(GL_TEXTURE_2D, mipmapTextureID);
    else glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFiltering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFiltering);
    update();
}

//----------------------------------------------------------------------------------------

void ModelViewer::initializeGL() {
    if(!this->context()->isValid()) {
        QMessageBox::critical(this, "CG Task 1", QString("Unable to initialize OpenGL"));
        qApp->exit(-1);
    }

    glewExperimental = true;
    GLenum initResult = glewInit();
    if (initResult != GLEW_OK) {
        QString errorStr = QString::fromUtf8((const char*)glewGetErrorString(initResult));
        QMessageBox::critical(this, "CG Task 1", QString("Unable to initialize OpenGL: %1").arg(errorStr));
        qApp->exit(-1);
    }

    std::cout << "OpenGL initialized: GL version "<< glGetString(GL_VERSION) << " | GLSL "<< glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
//    glEnable(GL_CULL_FACE);
    glPolygonOffset(-1.0, -1.0);

    shaderProgramID = createShaders(":/shaders/vertexShader.vsh", ":/shaders/fragmentShader.fsh");

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    mvpMatrixID = glGetUniformLocation(shaderProgramID, "MVP");
    drawOutlineID = glGetUniformLocation(shaderProgramID, "drawOutline");
    outlineColorID = glGetUniformLocation(shaderProgramID, "outlineColor");
    samplerID = glGetUniformLocation(shaderProgramID, "texSampler");
    uvMulID = glGetUniformLocation(shaderProgramID, "uvMul");
    drawMipLevelsID = glGetUniformLocation(shaderProgramID, "drawMipLevels");

//    generateRealMipmap(225, 225);
}

void ModelViewer::paintGL() {
    glClearColor(0, 0, 0.4f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(model) {
        QMatrix4x4 mMVP = mProjection * mView * mModel;
        GLfloat mGLMVP[16];
        qreal2glfloat(mMVP, mGLMVP);

        glUseProgram(shaderProgramID);
        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, mGLMVP);

        glActiveTexture(GL_TEXTURE0);
        if(drawRealMipmap) glBindTexture(GL_TEXTURE_2D, mipmapTextureID);
        else glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(samplerID, 0);

        glUniform1f(uvMulID, uvMul);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform1i(drawOutlineID, 0);
        glUniform1i(drawMipLevelsID, isDrawMipLevelsEnabled());
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDrawArrays(GL_TRIANGLES, 0, vertexBufferSize);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        if(drawOutline) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUniform1i(drawOutlineID, 1);
            glUniform3f(outlineColorID, (GLfloat)outlineColor.x(), (GLfloat)outlineColor.y(), (GLfloat)outlineColor.z());
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glDrawArrays(GL_TRIANGLES, 0, vertexBufferSize);
            glDisable(GL_POLYGON_OFFSET_FILL);
            glDisableVertexAttribArray(0);
        }
    }
}

void ModelViewer::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    mProjection.setToIdentity();
    mProjection.perspective(fovVal, (float)width / (float)height, pNear, pFar);
}

//----------------------------------------------------------------------------------------

void ModelViewer::mousePressEvent(QMouseEvent *event) {
    lastMousePos = event->pos();
}

void ModelViewer::mouseMoveEvent(QMouseEvent *event) {
    if(!(event->buttons() & Qt::LeftButton)) return;
    hAngle += event->x() - lastMousePos.x();
    vAngle += event->y() - lastMousePos.y();

    mModel.setToIdentity();
    mModel.rotate(hAngle, QVector3D(0, 1, 0));
//    mModel.rotate(vAngle, QVector3D(cos(hAngle/180.0*M_PI) < 0 ? -1 : 1, 0, 0));
    mModel.rotate(vAngle, QVector3D(1, 0, 0));

    update();

    lastMousePos = event->pos();
}

void ModelViewer::wheelEvent(QWheelEvent *event) {
    if(event->buttons() & Qt::LeftButton) {
        fovVal = std::min(std::max(fovVal - 0.05 * event->delta(), 20.0), 80.0);
        mProjection.setToIdentity();
        mProjection.perspective(fovVal, (float)this->width() / (float)this->height(), pNear, pFar);
    } else {
        zPos = std::min(std::max((double)pNear, zPos - 0.0125 * event->delta()), (double)pFar);
        mView.setToIdentity();
        mView.lookAt(QVector3D(0, 0, zPos), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    }
    update();
}

//----------------------------------------------------------------------------------------

int ModelViewer::isDrawMipLevelsEnabled() const {
    if(minFiltering != GL_NEAREST && minFiltering != GL_LINEAR && drawMipLevels) return 1;
    return 0;
}

void ModelViewer::resetView() {
    hAngle = 0;
    vAngle = 0;
    fovVal = 45.0;
    zPos = 15;

    mProjection.setToIdentity();
    mProjection.perspective(fovVal, (float)this->width() / (float)this->height(), pNear, pFar);
    mView.setToIdentity();
    mView.lookAt(QVector3D(0, 0, zPos), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    mModel.setToIdentity();
}

QString ModelViewer::readFile(const QString &fileName) const {
    QFile file(fileName);
    QString result = "";
    if(file.open(QFile::ReadOnly)) {
        QTextStream ts(&file);
        while(!ts.atEnd()) result += ts.readLine() + "\n";
        file.close();
    }
    return result;
}

GLuint ModelViewer::createShaders(const QString &vshFile, const QString &fshFile) const {
    std::string vertexShaderCode = readFile(vshFile).toStdString();
    const char *pVertexShaderCode = vertexShaderCode.c_str();
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderID, 1, &pVertexShaderCode, NULL);
    glCompileShader(vertexShaderID);
    if(!checkStatus(vertexShaderID, GL_COMPILE_STATUS)) return 0;

    std::string fragmentShaderCode = readFile(fshFile).toStdString();
    const char *pFragmentShaderCode = fragmentShaderCode.c_str();
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderID, 1, &pFragmentShaderCode, NULL);
    glCompileShader(fragmentShaderID);
    if(!checkStatus(fragmentShaderID, GL_COMPILE_STATUS)) return 0;

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShaderID);
    glAttachShader(shaderProgram, fragmentShaderID);
    glLinkProgram(shaderProgram);
    if(!checkStatus(shaderProgram, GL_LINK_STATUS, false)) return 0;

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    return shaderProgram;
}

bool ModelViewer::checkStatus(GLuint id, GLenum type, bool isShader) const {
    GLint result = GL_FALSE;
    if(isShader) glGetShaderiv(id, type, &result);
    else glGetProgramiv(id, type, &result);

    if(!result) {
        GLint infoLogLength = 0;
        if(isShader) glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
        else glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<char> errorMessage(infoLogLength + 1);
        glGetShaderInfoLog(id, infoLogLength, NULL, &errorMessage[0]);
        std::cout << errorMessage.data() << std::endl;
    }

    return result;
}
