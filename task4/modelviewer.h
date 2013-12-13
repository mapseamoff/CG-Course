#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include <GL/glew.h>

#include <QGLWidget>
#include <QMatrix4x4>
#include <QTimer>

#include "objmodel.h"

//----------------------------------------------------------------------------------------

class CubemapTexture {
public:
    CubemapTexture() : texID(0) {}
    ~CubemapTexture() {
        glDeleteTextures(1, &texID);
    }

    bool load(const QString &posX, const QString &negX,
              const QString &posY, const QString &negY,
              const QString &posZ, const QString &negZ) {
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

        QMap<GLenum, QString> files;
        files.insert(GL_TEXTURE_CUBE_MAP_POSITIVE_X, posX);
        files.insert(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, negX);
        files.insert(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, posY);
        files.insert(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, negY);
        files.insert(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, posZ);
        files.insert(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, negZ);

        for(QMap<GLenum, QString>::Iterator i = files.begin(); i != files.end(); ++i) {
            QImage tex = QImage(i.value()).convertToFormat(QImage::Format_RGB888);
            if(tex.isNull()) return false;
            glTexImage2D(i.key(), 0, GL_RGB, tex.width(), tex.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, tex.bits());
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return true;
    }

    bool load(const QList<QImage> &imgs) {
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

        for(int i = 0; i < 6; ++i) {
            const QImage &tex = imgs.at(i);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, tex.width(), tex.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, tex.bits());
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return true;
    }

    GLuint getTexID() const {
        return texID;
    }

private:
    GLuint texID;
};

//----------------------------------------------------------------------------------------

class ModelViewer : public QGLWidget {
    Q_OBJECT

public:
    ModelViewer(const QGLFormat &fmt, QWidget *parent = 0);
    ~ModelViewer();

    void setTerrainBox(const QString &modelPath,
                       const QString &posX, const QString &negX,
                       const QString &posY, const QString &negY,
                       const QString &posZ, const QString &negZ);
    void setTerrainBox(const QString &modelPath, const QList<QImage> &imgs);
    void initParticles(size_t count, const QString &texPath);
    void generateParticles(int cubeSize);

signals:
    void openGLInitialized();

public slots:
    void terrrainModelLoaded(bool status);
    void setDistanceThreshold(double val);
    void setShowTerrain(bool val);
    void setBillboardType(int val);

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
    GLuint timeID, maxDistID, cubeSizeID;

    GLfloat pNear, pFar;
    QMatrix4x4 mProjection, mModel, mView;
    QVector3D vCameraPos, vCameraDir, vCameraUp, vCameraRight;
    QPoint lastMousePos;
    float hAngle, vAngle, fovVal;
    float distThreshold, psCubeSize;
    int billboardType;

    qint64 startTime, lastTime;
    MoveDir currentMoveDir;
    bool psEnabled, trEnabled, showTerrain;

    GLuint boxVertexBuffer, boxVertexBufferSize;
    GLuint boxShaderProgramID, boxMVPID, boxSamplerID;

    OBJModel *terrainModel;
    CubemapTexture terrainTex;

    size_t maxParticles;
    QTimer *psUpdateTimer;

};

#endif // MODELVIEWER_H
