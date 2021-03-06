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
    void setLighModel(OBJModel *lm);

signals:
    void uvMultiplierChanged(double val);

public slots:
    void setMeshColor(QVector3D mc);
    void setLightColor(QVector3D c);
    void setAmbientColor(QVector3D c);
    void setDiffuseColor(QVector3D c);
    void setSpecularColor(QVector3D c);
    void setSpecularPower(double p);
    void setLightPosition(QVector3D p);
    void setLightDirection(QVector3D p, QVector3D d);
    void setLightCutoff(double angle);
    void setLightExponent(double e);
    void setLightPower(double p);
    void setFillMethod(int m);
    void setShadingMethod(int m);
    void setSpotMethod(bool m);
    void setDrawOutline(bool val);
    void setDrawLightCone(bool val);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    QString readFile(const QString &fileName) const;
    GLuint createShaders(const QString &vshFile, const QString &fshFile, const QString &gshFile = "") const;
    bool checkStatus(GLuint id, GLenum type, bool isShader = true) const;
    void resetView();
    void updateLight();

    QQuaternion rotationBetweenVectors(const QVector3D &start, const QVector3D &dest) const;

    OBJModel *model, *lightModel;
    GLuint shaderProgramID, mvpMatrixID, mMatrixID, vMatrixID;
    GLuint fillMethodID, shadingMethodID;
    GLuint lightPosID, lightColorID, lightPowerID, lightDirID, lightAngleID, lightExponentID, spotMethodID;
    GLuint drawOutlineID, outlineColorID;
    GLuint ambientColorID, diffuseColorID, specularColorID, specularPowerID;
    GLuint vertexBuffer, vertexBufferSize, vertexArrayID;
    GLuint normalsBuffer;
    GLfloat pNear, pFar, specularPower, lightPower, lightAngle, lightExponent;
    QMatrix4x4 mProjection, mModel, mView;
    QVector3D outlineColor, ambientColor, diffuseColor, specularColor;
    QVector3D lightPosition, lightColor, lightDirection, lightPointsAt;
    QPoint lastMousePos;
    float hAngle, vAngle, mScale;
    float fovVal, zPos;
    bool drawOutline, drawLightCone;
    int fillMethod, shadingMethod, spotMethod;

    GLuint lightVertexBuffer, lightVertexBufferSize, lightVertexArrayID;
    QMatrix4x4 mLightModel;

};

#endif // MODELVIEWER_H
