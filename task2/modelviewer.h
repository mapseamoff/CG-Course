#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include <GL/glew.h>

#include <QGLWidget>
#include <QMatrix4x4>

#include "objmodel.h"

class ModelViewer : public QGLWidget {
    Q_OBJECT

public:
    ModelViewer(const QGLFormat &fmt, QWidget *parent = 0);
    ~ModelViewer();

    void setModel(OBJModel *m);

signals:
    void uvMultiplierChanged(double val);

public slots:
    void setMeshColor(QVector3D mc);
    void setTexCoordsMultiplier(double val);
    void setFilteringType(int ft);
    void setDrawOutline(bool val);
    void setDrawMipLevels(bool val);
    void setDrawRealMipmap(bool val);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    QString readFile(const QString &fileName) const;
    GLuint createShaders(const QString &vshFile, const QString &fshFile) const;
    bool checkStatus(GLuint id, GLenum type, bool isShader = true) const;
    void resetView();
    int isDrawMipLevelsEnabled() const;
    void generateRealMipmap();

    OBJModel *model;
    GLuint shaderProgramID, mvpMatrixID, samplerID, textureID, mipmapTextureID;
    GLuint drawOutlineID, outlineColorID, uvMulID;
    GLuint vertexBuffer, vertexBufferSize, vertexArrayID;
    GLuint uvBuffer;
    GLuint drawMipLevelsID;
    GLint minFiltering, magFiltering;
    GLfloat pNear, pFar, uvMul;
    QMatrix4x4 mProjection, mModel, mView;
    QVector3D outlineColor;
    QPoint lastMousePos;
    float hAngle, vAngle;
    float fovVal, zPos;
    bool drawOutline, drawMipLevels, drawRealMipmap;

};

#endif // MODELVIEWER_H
