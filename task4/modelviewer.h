#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include <GL/glew.h>

#include <QGLWidget>
#include <QMatrix4x4>
#include <QTimer>

#include "terrain.h"

//----------------------------------------------------------------------------------------

class ModelViewer : public QGLWidget {
    Q_OBJECT

public:
    ModelViewer(const QGLFormat &fmt, QWidget *parent = 0);
    ~ModelViewer();

    void setTerrainBox(const QList<QImage> &imgs);
    void setTerrainBox(const QString &cubemap);
    void setFrustumModel(const QString &model);
    void initParticles(size_t count, const QString &texPath);
    void initTerrain(int cubeSize, int gridSize);
    void generateParticles(int cubeSize);
    void generateTerrain(float persistence, float frequency, float amplitude, int octaves);

    void resetView();

signals:
    void openGLInitialized();

public slots:
    void setDistanceThreshold(double val);
    void setShowTerrain(bool val);
    void setBillboardType(int val);
    void setWireframeMode(bool val);
    void setTerrainTexMode(int val);
    void setTerrainContrast(double val);

    void setCurrentCamera(int i);
    void setCameraMode(bool single);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *);
    void wheelEvent(QWheelEvent *event);
    void focusOutEvent(QFocusEvent *);

private:
    enum MoveDir { None, Left, Right, Forward, Backward };

    struct Camera {
        QVector3D pos, dir, up, right;
    };

    QString readFile(const QString &fileName) const;
    GLuint createShaders(const QString &vshFile, const QString &fshFile, const QString &gshFile = "") const;
    bool checkStatus(GLuint id, GLenum type, bool isShader = true) const;

    void renderParticleSystemPrecomp(bool wireframe, bool top);

    void updateCameraPos(qint64 deltaTime);
    void updateCameraFrustum();
    void findIntersectedOctants();
    Camera &currentCamera();

    QVector3D getShiftForOctant(int i) const;

    GLuint particlesPosBuffer, particlesSpeedBuffer, particlesDelayBuffer[2];
    GLuint particleTexID, vertexArrayID;

    GLuint shaderProgramID, vpMatrixID, texSamplerID;
    GLuint cameraPosID, cameraRightID, cameraUpID;
    GLuint viewportSizeID, billboardTypeID;
    GLuint timeID, maxDistID, cubeSizeID, psWireframeID, shiftID;

    GLfloat pNear, pFar;
    QMatrix4x4 mProjection, mModel, mView, pVP;
    Camera vCamera, fCamera;
    QPoint lastMousePos;
    float hAngle, vAngle, fovVal;
    float distThreshold, psCubeSize, terrainContrast;
    int billboardType, terrainTexMode, currentCameraID;

    qint64 startTime, lastTime;
    MoveDir currentMoveDir;
    bool psEnabled, trEnabled, showTerrain, showWireframe;

    GLuint boxShaderProgramID;
    Skybox skybox;

    GLuint terrainShaderProgramID;
    Terrain terrain;

    GLuint frustumShaderProgramID;
    CameraFrustum vFrustum;

    size_t maxParticles;
    QTimer *psUpdateTimer;

};

#endif // MODELVIEWER_H
