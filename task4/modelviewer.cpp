#include "modelviewer.h"

#include <QFile>
#include <QMouseEvent>
#include <QApplication>
#include <QMessageBox>

#include <math.h>

#include <iostream>

#include <QDateTime>

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

ModelViewer::ModelViewer(const QGLFormat &fmt, QWidget *parent) : QGLWidget(new QGLContext(fmt), parent) {
    hAngle = 0;
    vAngle = 0;
    fovVal = 45.0;
    pNear = 0.1;
    pFar = 500.0;
    distThreshold = 50.0;
    billboardType = 0;
    psEnabled = false;
    trEnabled = false;
    showTerrain = true;
    currentMoveDir = None;

    terrainModel = new OBJModel(this);

    psUpdateTimer = new QTimer(this);
    connect(psUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
}

ModelViewer::~ModelViewer() {
    glDeleteProgram(shaderProgramID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteBuffers(1, &particlesPosBuffer);
    glDeleteBuffers(1, &particlesSpeedBuffer);
    glDeleteTextures(1, &particleTexID);
}

//----------------------------------------------------------------------------------------

void ModelViewer::terrrainModelLoaded(bool status) {
    trEnabled = status;

    std::vector<OBJVec3> vs;
    for(std::vector<OBJFace>::iterator fi = terrainModel->faces.begin(); fi != terrainModel->faces.end(); ++fi) {
        for(OBJFace::iterator fii = fi->begin(); fii != fi->end(); ++fii) {
            vs.push_back(terrainModel->verts[fii->v - 1]);
        }
    }

    glGenBuffers(1, &boxVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, boxVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(OBJVec3), &vs[0], GL_STATIC_DRAW);
    boxVertexBufferSize = vs.size();

    resetView();
    update();
}

void ModelViewer::setDistanceThreshold(double val) {
    distThreshold = val;
    update();
}

void ModelViewer::setShowTerrain(bool val) {
    showTerrain = val;
    update();
}

void ModelViewer::setBillboardType(int val) {
    billboardType = val;
    update();
}

//----------------------------------------------------------------------------------------

void ModelViewer::setTerrainBox(const QString &modelPath, const QString &posX, const QString &negX, const QString &posY, const QString &negY, const QString &posZ, const QString &negZ) {
    terrainModel->loadModel(modelPath);
    connect(terrainModel, SIGNAL(loadStatus(bool)), this, SLOT(terrrainModelLoaded(bool)));
    terrainTex.load(posX, negX, posY, negY, posZ, negZ);
}

void ModelViewer::setTerrainBox(const QString &modelPath, const QList<QImage> &imgs) {
    terrainModel->loadModel(modelPath);
    connect(terrainModel, SIGNAL(loadStatus(bool)), this, SLOT(terrrainModelLoaded(bool)));
    terrainTex.load(imgs);
}

//----------------------------------------------------------------------------------------

void ModelViewer::initParticles(size_t count, const QString &texPath) {
    maxParticles = count;

    if(psEnabled) {
        glDeleteBuffers(1, &particlesPosBuffer);
        glDeleteBuffers(1, &particlesSpeedBuffer);
        glDeleteTextures(1, &particleTexID);
    }

    psEnabled = false;

    QImage particleTex = QImage(texPath).convertToFormat(QImage::Format_RGB888);
    glGenTextures(1, &particleTexID);
    glBindTexture(GL_TEXTURE_2D, particleTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, particleTex.width(), particleTex.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, particleTex.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void ModelViewer::generateParticles(int cubeSize) {
    psCubeSize = cubeSize;
    int halfSize = cubeSize / 2;

    qsrand(QDateTime::currentMSecsSinceEpoch());
    std::vector<GLfloat> vs;
    std::vector<GLfloat> sp;
    for(size_t i = 0; i < maxParticles; ++i) {
        vs.push_back(qrand() % cubeSize - halfSize);        //pos x
        vs.push_back(qrand() % halfSize + halfSize / 2);    //pos y
        vs.push_back(qrand() % cubeSize - halfSize);        //pos z
        vs.push_back(qrand() % 30 + 10);                    //size
//        vs.push_back(qrand() % 3 + 1.0);                    //size
        sp.push_back((qrand() % 300 + 200.0) / 10000.0);    //speed range 0.02 .. 0.05
        sp.push_back(qrand() % 20);                         //radius
        sp.push_back(float(qrand() % 200) / 100000.0);      //freq
    }

    glGenBuffers(1, &particlesPosBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, particlesPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(GLfloat), &vs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &particlesSpeedBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, particlesSpeedBuffer);
    glBufferData(GL_ARRAY_BUFFER, sp.size() * sizeof(GLfloat), &sp[0], GL_STATIC_DRAW);

    psEnabled = true;
    startTime = QDateTime::currentMSecsSinceEpoch();

    psUpdateTimer->start(16); // ~ 60 fps
    resetView();
    update();
}

//----------------------------------------------------------------------------------------

void ModelViewer::resetView() {
    hAngle = 180.0;
    vAngle = 0;
    fovVal = 60.0;

    vCameraPos = QVector3D(0, 0, 0);
    vCameraUp = QVector3D(0, 1, 0);
    vCameraDir = QVector3D(0, 0, -1);
    vCameraRight = QVector3D(1, 0, 0);

    mProjection.setToIdentity();
    mProjection.perspective(fovVal, (float)this->width() / (float)this->height(), pNear, pFar);
    mView.setToIdentity();
    mView.lookAt(vCameraPos, vCameraPos + vCameraDir, vCameraUp);
    mModel.setToIdentity();
}

void ModelViewer::updateCameraPos(qint64 deltaTime) {
    static const float mspeed = 0.1;
    bool changed = true;

    switch(currentMoveDir) {
    case Forward: vCameraPos += vCameraDir * deltaTime * mspeed; break;
    case Backward: vCameraPos -= vCameraDir * deltaTime * mspeed; break;
    case Right: vCameraPos += vCameraRight * deltaTime * mspeed; break;
    case Left: vCameraPos -= vCameraRight * deltaTime * mspeed; break;
    default: changed = false; break;
    }

    if(changed) {
        mView.setToIdentity();
        mView.lookAt(vCameraPos, vCameraPos + vCameraDir, vCameraUp);
    }
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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);

//    shaderProgramID = createShaders(":/shaders/vertexShader.vsh", ":/shaders/fragmentShader.fsh", ":/shaders/geometryShader.geom");
    shaderProgramID = createShaders(":/shaders/particleVS.vsh", ":/shaders/particleFS.fsh", ":/shaders/particleGS.gsh");

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    vpMatrixID = glGetUniformLocation(shaderProgramID, "VP");
    cameraPosID = glGetUniformLocation(shaderProgramID, "cameraPos");
    cameraRightID = glGetUniformLocation(shaderProgramID, "cameraRight");
    cameraUpID = glGetUniformLocation(shaderProgramID, "cameraUp");
    viewportSizeID = glGetUniformLocation(shaderProgramID, "viewportSize");
    billboardTypeID = glGetUniformLocation(shaderProgramID, "billboardType");
    texSamplerID = glGetUniformLocation(shaderProgramID, "texSampler");
    timeID = glGetUniformLocation(shaderProgramID, "time");
    maxDistID = glGetUniformLocation(shaderProgramID, "maxDist");
    cubeSizeID = glGetUniformLocation(shaderProgramID, "cubeSize");

    boxShaderProgramID = createShaders(":/shaders/boxVS.vsh", ":/shaders/boxFS.fsh");
    boxMVPID = glGetUniformLocation(boxShaderProgramID, "MVP");
    boxSamplerID = glGetUniformLocation(boxShaderProgramID, "cubemapSampler");

    emit openGLInitialized();
}

void ModelViewer::paintGL() {
    glClearColor(0, 0, 0.0f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(trEnabled && showTerrain) {
        GLint oldCFMode, oldDFMode;
        glGetIntegerv(GL_CULL_FACE_MODE, &oldCFMode);
        glGetIntegerv(GL_DEPTH_FUNC, &oldDFMode);

        glCullFace(GL_FRONT);
        glDepthFunc(GL_LEQUAL);

        mModel.setToIdentity();
        mModel.translate(vCameraPos);

        QMatrix4x4 mMVP = mProjection * mView * mModel;

        glUseProgram(boxShaderProgramID);
        setUniformMatrix(glUniformMatrix4fv, boxMVPID, mMVP, 4, 4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, terrainTex.getTexID());
        glUniform1i(boxSamplerID, 0);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, boxVertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDrawArrays(GL_TRIANGLES, 0, boxVertexBufferSize);
        glDisableVertexAttribArray(0);

        glCullFace(oldCFMode);
        glDepthFunc(oldDFMode);
    }

    if(psEnabled) {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        qint64 deltaTime =  currentTime - lastTime;
        lastTime = currentTime;

        updateCameraPos(deltaTime);
        QMatrix4x4 mVP = mProjection * mView;

        glUseProgram(shaderProgramID);

        setUniformMatrix(glUniformMatrix4fv, vpMatrixID, mVP, 4, 4);
        setUniformVector3f(cameraRightID, vCameraRight);
        setUniformVector3f(cameraPosID, vCameraPos);
        setUniformVector3f(cameraUpID, vCameraUp);
        glUniform1i(timeID, (int)(currentTime - startTime));
        glUniform1i(billboardTypeID, billboardType);
        glUniform2f(viewportSizeID, (GLfloat)this->size().width(), (GLfloat)this->size().height());
        glUniform1f(maxDistID, distThreshold);
        glUniform1f(cubeSizeID, psCubeSize);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, particleTexID);
        glUniform1i(texSamplerID, 0);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, particlesPosBuffer);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, particlesSpeedBuffer);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDrawArrays(GL_POINTS, 0, maxParticles);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }

}

void ModelViewer::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    mProjection.setToIdentity();
    mProjection.perspective(fovVal, (float)width / (float)height, pNear, pFar);
}

//----------------------------------------------------------------------------------------

void ModelViewer::mousePressEvent(QMouseEvent *event) {
    this->setFocus();
    lastMousePos = event->pos();
}

void ModelViewer::mouseMoveEvent(QMouseEvent *event) {
    if(!(event->buttons() & Qt::LeftButton)) return;
    hAngle += event->x() - lastMousePos.x();
    vAngle = qMin(90.0f, qMax(-90.0f, vAngle + event->y() - lastMousePos.y()));
    lastMousePos = event->pos();

    float rhAngle = hAngle / 180.0 * M_PI;
    float rvAngle = vAngle / 180.0 * M_PI;

    vCameraDir = QVector3D(cos(rvAngle) * sin(rhAngle), sin(rvAngle), cos(rvAngle) * cos(rhAngle));
    vCameraRight = QVector3D(sin(rhAngle - M_PI_2), 0, cos(rhAngle - M_PI_2));
    vCameraUp = QVector3D::crossProduct(vCameraRight, vCameraDir);

    mView.setToIdentity();
    mView.lookAt(vCameraPos, vCameraPos + vCameraDir, vCameraUp);

    update();
}

void ModelViewer::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Up || event->key() == Qt::Key_W) {
        currentMoveDir = Forward;
    } else if(event->key() == Qt::Key_Down || event->key() == Qt::Key_S) {
        currentMoveDir = Backward;
    } else if(event->key() == Qt::Key_Left || event->key() == Qt::Key_A) {
        currentMoveDir = Left;
    } else if(event->key() == Qt::Key_Right || event->key() == Qt::Key_D) {
        currentMoveDir = Right;
    }
}

void ModelViewer::keyReleaseEvent(QKeyEvent *) {
    currentMoveDir = None;
}

void ModelViewer::wheelEvent(QWheelEvent *event) {
    if(event->buttons() & Qt::RightButton) {
        fovVal = std::min(std::max(fovVal - 0.05 * event->delta(), 10.0), 90.0);
        mProjection.setToIdentity();
        mProjection.perspective(fovVal, (float)this->width() / (float)this->height(), pNear, pFar);
    }
    update();
}

void ModelViewer::focusOutEvent(QFocusEvent *) {
    currentMoveDir = None;
}

//----------------------------------------------------------------------------------------

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
