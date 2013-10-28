#include "modelviewer.h"

#include <QFile>
#include <QMouseEvent>
#include <QApplication>
#include <QMessageBox>

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
    depthFillMethod = 0;
    pNear = 0.1;
    pFar = 100.0;
    outlineColor = QVector3D(0, 0, 0);
}

ModelViewer::~ModelViewer() {
    model = 0;
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteProgram(shaderProgramID);
    glDeleteVertexArrays(1, &vertexArrayID);
}

void ModelViewer::setModel(OBJModel *m) {
    if(model) {
        glDeleteBuffers(1, &vertexBuffer);
    }

    std::vector<OBJVec3> vs, ns, ts;
    for(std::vector<OBJFace>::iterator fi = m->faces.begin(); fi != m->faces.end(); ++fi) {
        for(OBJFace::iterator fii = fi->begin(); fii != fi->end(); ++fii) {
            if(fii->n != 0) ns.push_back(m->norms[fii->n - 1]);
            if(fii->t != 0) ts.push_back(m->texs[fii->t - 1]);
            vs.push_back(m->verts[fii->v - 1]);
        }
    }

    //assume there is only triangles in the model
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(OBJVec3), &vs[0], GL_STATIC_DRAW);
    vertexBufferSize = vs.size();

    model = m;

    resetView();
}

void ModelViewer::setOutlineColor(double r, double g, double b) {
    outlineColor = QVector3D(r, g, b);
    update();
}

void ModelViewer::setFillMethod(int m) {
    if(depthFillMethod != m) {
        depthFillMethod = m;
        update();
    }
}

void ModelViewer::setNearPlane(double val) {
    pNear = val;
    update();
}

void ModelViewer::setFarPlane(double val) {
    pFar = val;
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
    glEnable(GL_CULL_FACE);
    glPolygonOffset(-1.0, -1.0);

    shaderProgramID = createShaders(":/vertexShader.vsh", ":/fragmentShader.fsh");

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    mvpMatrixID = glGetUniformLocation(shaderProgramID, "MVP");
    invpMatrixID = glGetUniformLocation(shaderProgramID, "invP");
    drawOutlineID = glGetUniformLocation(shaderProgramID, "drawOutline");
    depthFillMethodID = glGetUniformLocation(shaderProgramID, "depthFillMethod");
    outlineColorID = glGetUniformLocation(shaderProgramID, "outlineColor");
    nearID = glGetUniformLocation(shaderProgramID, "near");
    farID = glGetUniformLocation(shaderProgramID, "far");
}

void ModelViewer::paintGL() {
    glClearColor(0, 0, 0.4f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(model) {
        QMatrix4x4 mMVP = mProjection * mView * mModel;
        QMatrix4x4 invP = mProjection.inverted();

        glUseProgram(shaderProgramID);
        GLfloat mGLInvP[16], mGLMVP[16];
        qreal2glfloat(invP, mGLInvP);
        qreal2glfloat(mMVP, mGLMVP);

        glUniformMatrix4fv(invpMatrixID, 1, GL_FALSE, mGLInvP);
        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, mGLMVP);
        glUniform1f(nearID, pNear);
        glUniform1f(farID, pFar);

//        setUniformMatrix(glUniformMatrix4fv, invpMatrixID, invP, 4, 4);
//        setUniformMatrix(glUniformMatrix4fv, mvpMatrixID, mMVP, 4, 4);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUniform1i(drawOutlineID, 0);
        glUniform1i(depthFillMethodID, depthFillMethod);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDrawArrays(GL_TRIANGLES, 0, vertexBufferSize);
        glDisableVertexAttribArray(0);

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
    if(event->buttons() & Qt::LeftButton & Qt::RightButton) {
        fovVal = std::min(std::max(fovVal - 0.05 * event->delta(), 20.0), 80.0);
        mProjection.setToIdentity();
        mProjection.perspective(fovVal, (float)this->width() / (float)this->height(), pNear, pFar);
    } else if(event->buttons() & Qt::LeftButton) {
        pFar = std::max(0.1, std::min(pFar + 0.025 * event->delta(), 1E3));
        emit farPlaneChanged(pFar);
    } else if(event->buttons() & Qt::RightButton) {
        pNear = std::max(0.1, std::min(pNear + 0.025 * event->delta(), 1E3));
        emit nearPlaneChanged(pNear);
    } else {
        zPos = std::min(std::max((double)pNear, zPos - 0.0125 * event->delta()), (double)pFar);
        mView.setToIdentity();
        mView.lookAt(QVector3D(0, 0, zPos), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    }
    update();
}

//----------------------------------------------------------------------------------------

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
