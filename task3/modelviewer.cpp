#include "modelviewer.h"

#include <QFile>
#include <QMouseEvent>
#include <QApplication>
#include <QMessageBox>

#include <math.h>

#include <iostream>

//----------------------------------------------------------------------------------------

// We have to repack matrices from qreal to GLfloat.
#if QT_VERSION >= 0x050000
#define setUniformMatrix(func,location,value,cols,rows) \
        { \
        GLfloat mat[cols * rows]; \
        const float *data = value.constData(); \
        for (int i = 0; i < cols * rows; ++i) mat[i] = data[i]; \
        func(location, 1, GL_FALSE, mat); \
        }
#else
#define setUniformMatrix(func,location,value,cols,rows) \
        { \
        GLfloat mat[cols * rows]; \
        const qreal *data = value.constData(); \
        for (int i = 0; i < cols * rows; ++i) mat[i] = data[i]; \
        func(location, 1, GL_FALSE, mat); \
        }
#endif

#define setUniformVector3f(location, value) \
    glUniform3f(location, (GLfloat)value.x(), (GLfloat)value.y(), (GLfloat)value.z());

ModelViewer::ModelViewer(const QGLFormat &fmt, QWidget *parent) : QGLWidget(new QGLContext(fmt), parent), model(0) {
    hAngle = 0;
    vAngle = 0;
    fovVal = 45.0;
    zPos = 10;
    pNear = 0.1;
    pFar = 100.0;
    mScale = 5.0;
    outlineColor = QVector3D(0, 0, 0);
    drawOutline = true;
    drawMipLevels = false;
    drawRealMipmap = false;
    lightPosition = QVector3D(4, 4, 4);
    lightDirection = -lightPosition.normalized();
    specularPower = 20.0;
    lightPower = 50.0;
    fillMethod = 0;
    shadingMethod = 0;
    spotMethod = 1;
    lightAngle = cos(M_PI * 45.0 / 180.0);
    lightExponent = 1.0;
}

ModelViewer::~ModelViewer() {
    model = 0;
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &normalsBuffer);
    glDeleteProgram(shaderProgramID);
    glDeleteVertexArrays(1, &vertexArrayID);
}

void ModelViewer::setModel(OBJModel *m) {
    if(model) {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &normalsBuffer);
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

    glGenBuffers(1, &normalsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
    glBufferData(GL_ARRAY_BUFFER, ns.size() * sizeof(OBJVec3), &ns[0], GL_STATIC_DRAW);

    model = m;
    resetView();
    update();
}

void ModelViewer::setMeshColor(QVector3D mc) {
    outlineColor = mc;
    update();
}

void ModelViewer::setDrawOutline(bool val) {
    drawOutline = val;
    update();
}

void ModelViewer::setAmbientColor(QVector3D c) {
    ambientColor = c;
    update();
}

void ModelViewer::setDiffuseColor(QVector3D c) {
    diffuseColor = c;
    update();
}

void ModelViewer::setSpecularColor(QVector3D c) {
    specularColor = c;
    update();
}

void ModelViewer::setSpecularPower(double p) {
    specularPower = p;
    update();
}

void ModelViewer::setLightColor(QVector3D c) {
    lightColor = c;
    update();
}

void ModelViewer::setLightPosition(QVector3D p) {
    lightPosition = p;
    update();
}

void ModelViewer::setFillMethod(int m) {
    fillMethod = m;
    update();
}

void ModelViewer::setShadingMethod(int m) {
    shadingMethod = m;
    update();
}

void ModelViewer::setLightPower(double p) {
    lightPower = p;
    update();
}

void ModelViewer::setLightDirection(QVector3D p) {
    lightDirection = p;
    update();
}

void ModelViewer::setLightCutoff(double angle) {
    lightAngle = cos(M_PI * angle / 180.0);
    update();
}

void ModelViewer::setLightExponent(double e) {
    lightExponent = e;
    update();
}

void ModelViewer::setSpotMethod(bool m) {
    spotMethod = m ? 1 : 0;
    update();
}

//----------------------------------------------------------------------------------------

void ModelViewer::initializeGL() {
    if(!this->context()->isValid()) {
        QMessageBox::critical(this, "CG Task 3", QString("Unable to initialize OpenGL"));
        qApp->exit(-1);
    }

    glewExperimental = true;
    GLenum initResult = glewInit();
    if (initResult != GLEW_OK) {
        QString errorStr = QString::fromUtf8((const char*)glewGetErrorString(initResult));
        QMessageBox::critical(this, "CG Task 3", QString("Unable to initialize OpenGL: %1").arg(errorStr));
        qApp->exit(-1);
    }

    std::cout << "OpenGL initialized: GL version "<< glGetString(GL_VERSION) << " | GLSL "<< glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
//    glEnable(GL_CULL_FACE);
    glPolygonOffset(-1.0, -1.0);

    shaderProgramID = createShaders(":/shaders/vertexShader.vsh", ":/shaders/fragmentShader.fsh", ":/shaders/geometryShader.geom");
//    shaderProgramID = createShaders(":/shaders/vertexShader.vsh", ":/shaders/fragmentShader.fsh");

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    mvpMatrixID = glGetUniformLocation(shaderProgramID, "MVP");
    mMatrixID = glGetUniformLocation(shaderProgramID, "M");
    vMatrixID = glGetUniformLocation(shaderProgramID, "V");

    lightPosID = glGetUniformLocation(shaderProgramID, "lightPosition_worldspace");
    lightColorID = glGetUniformLocation(shaderProgramID, "lightColor");
    lightPowerID = glGetUniformLocation(shaderProgramID, "lightPower");
    lightDirID = glGetUniformLocation(shaderProgramID, "spotDirection_worldspace");
    lightAngleID = glGetUniformLocation(shaderProgramID, "spotAngleCos");
    lightExponentID = glGetUniformLocation(shaderProgramID, "spotExponent");
    spotMethodID = glGetUniformLocation(shaderProgramID, "spotMethod");

    fillMethodID = glGetUniformLocation(shaderProgramID, "fillMethod");
    shadingMethodID = glGetUniformLocation(shaderProgramID, "shadingMethod");
    drawOutlineID = glGetUniformLocation(shaderProgramID, "drawOutline");
    outlineColorID = glGetUniformLocation(shaderProgramID, "outlineColor");
    ambientColorID = glGetUniformLocation(shaderProgramID, "ambientColor");
    diffuseColorID = glGetUniformLocation(shaderProgramID, "diffuseColor");
    specularColorID = glGetUniformLocation(shaderProgramID, "specularColor");
    specularPowerID = glGetUniformLocation(shaderProgramID, "specularPower");
}

void ModelViewer::paintGL() {
    glClearColor(0, 0, 0.4f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(model) {
        QMatrix4x4 mMVP = mProjection * mView * mModel;

        glUseProgram(shaderProgramID);

        setUniformMatrix(glUniformMatrix4fv, mvpMatrixID, mMVP, 4, 4);
        setUniformMatrix(glUniformMatrix4fv, mMatrixID, mModel, 4, 4);
        setUniformMatrix(glUniformMatrix4fv, vMatrixID, mView, 4, 4);

        setUniformVector3f(lightPosID, lightPosition);
        setUniformVector3f(lightColorID, lightColor);
        setUniformVector3f(ambientColorID, ambientColor);
        setUniformVector3f(diffuseColorID, diffuseColor);
        setUniformVector3f(specularColorID, specularColor);
        setUniformVector3f(lightDirID, lightDirection);
        glUniform1f(specularPowerID, specularPower);
        glUniform1f(lightPowerID, lightPower);
        glUniform1f(lightAngleID, lightAngle);
        glUniform1f(lightExponentID, lightExponent);
        glUniform1i(fillMethodID, fillMethod);
        glUniform1i(shadingMethodID, shadingMethod);
        glUniform1i(spotMethodID, spotMethod);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform1i(drawOutlineID, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDrawArrays(GL_TRIANGLES, 0, vertexBufferSize);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        if(drawOutline) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUniform1i(drawOutlineID, 1);
            setUniformVector3f(outlineColorID, outlineColor);
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
    mModel.scale(mScale);

    update();

    lastMousePos = event->pos();
}

void ModelViewer::wheelEvent(QWheelEvent *event) {
    if(event->buttons() & Qt::RightButton) {
        fovVal = std::min(std::max(fovVal - 0.05 * event->delta(), 10.0), 90.0);
        mProjection.setToIdentity();
        mProjection.perspective(fovVal, (float)this->width() / (float)this->height(), pNear, pFar);
    } else if(event->buttons() & Qt::LeftButton) {
        mScale = qMax(1.0, qMin(100.0, mScale + 0.05 * event->delta()));
        mModel.setToIdentity();
        mModel.rotate(hAngle, QVector3D(0, 1, 0));
        mModel.rotate(vAngle, QVector3D(1, 0, 0));
        mModel.scale(mScale);
    } else {
        zPos = std::min(std::max((double)pNear, zPos - 0.0025 * event->delta()), (double)pFar);
        mView.setToIdentity();
        mView.lookAt(QVector3D(0, 0, zPos), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    }
    update();
}

//----------------------------------------------------------------------------------------

void ModelViewer::resetView() {
    hAngle = 0;
    vAngle = 0;
    fovVal = 60.0;
    zPos = 4;
    mScale = 5.0;

    mProjection.setToIdentity();
    mProjection.perspective(fovVal, (float)this->width() / (float)this->height(), pNear, pFar);
    mView.setToIdentity();
    mView.lookAt(QVector3D(0, 0, zPos), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    mModel.setToIdentity();
    mModel.scale(mScale);
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

GLuint ModelViewer::createShaders(const QString &vshFile, const QString &fshFile, const QString &gshFile) const {
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

    GLuint geometryShaderID = 0;
    if(!gshFile.isEmpty()) {
        std::string geometryShaderCode = readFile(gshFile).toStdString();
        const char *pGeometryShaderCode = geometryShaderCode.c_str();
        geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShaderID, 1, &pGeometryShaderCode, NULL);
        glCompileShader(geometryShaderID);
        if(!checkStatus(geometryShaderID, GL_COMPILE_STATUS)) return 0;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShaderID);
    glAttachShader(shaderProgram, fragmentShaderID);
    if(!gshFile.isEmpty()) glAttachShader(shaderProgram, geometryShaderID);
    glLinkProgram(shaderProgram);
    if(!checkStatus(shaderProgram, GL_LINK_STATUS, false)) return 0;

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    if(!gshFile.isEmpty()) glDeleteShader(geometryShaderID);
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
        if(isShader) glGetShaderInfoLog(id, infoLogLength, NULL, &errorMessage[0]);
        else glGetProgramInfoLog(id, infoLogLength, NULL, &errorMessage[0]);
        std::cout << errorMessage.data() << std::endl;
    }

    return result;
}
