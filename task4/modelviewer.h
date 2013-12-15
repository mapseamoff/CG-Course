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
    void initParticles(size_t count, const QString &texPath);
    void initTerrain(int cubeSize, int gridSize);
    void generateParticles(int cubeSize);
    void generateTerrain(float persistence, float frequency, float amplitude, int octaves);

signals:
    void openGLInitialized();

public slots:
    void setDistanceThreshold(double val);
    void setShowTerrain(bool val);
    void setBillboardType(int val);
    void setWireframeMode(bool val);

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

    QString readFile(const QString &fileName) const;
    GLuint createShaders(const QString &vshFile, const QString &fshFile, const QString &gshFile = "") const;
    bool checkStatus(GLuint id, GLenum type, bool isShader = true) const;
    void resetView();
    void updateCameraPos(qint64 deltaTime);

    GLuint particlesPosBuffer, particlesSpeedBuffer;
    GLuint particleTexID, vertexArrayID;

    GLuint shaderProgramID, vpMatrixID, texSamplerID;
    GLuint cameraPosID, cameraRightID, cameraUpID;
    GLuint viewportSizeID, billboardTypeID;
    GLuint timeID, maxDistID, cubeSizeID, psWireframeID;

    GLfloat pNear, pFar;
    QMatrix4x4 mProjection, mModel, mView;
    QVector3D vCameraPos, vCameraDir, vCameraUp, vCameraRight;
    QPoint lastMousePos;
    float hAngle, vAngle, fovVal;
    float distThreshold, psCubeSize;
    int billboardType;

    qint64 startTime, lastTime;
    MoveDir currentMoveDir;
    bool psEnabled, trEnabled, showTerrain, showWireframe;

    GLuint boxShaderProgramID;
    Skybox skybox;

    GLuint terrainShaderProgramID;
    Terrain terrain;

    size_t maxParticles;
    QTimer *psUpdateTimer;

};

#endif // MODELVIEWER_H
