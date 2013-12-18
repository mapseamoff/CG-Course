#ifndef TERRAIN_H
#define TERRAIN_H

#include <GL/glew.h>

#include <QList>
#include <QImage>
#include <QMatrix4x4>

#include "objmodel.h"

class CubemapTexture {
public:
    CubemapTexture() : texID(0) {}
    ~CubemapTexture();

    // returns negY texture
    QImage load(const QString &posX, const QString &negX,
              const QString &posY, const QString &negY,
              const QString &posZ, const QString &negZ);
    QImage load(const QList<QImage> &imgs);
    QImage load(const QString &cubemap);

    GLuint getTexID() const {
        return texID;
    }

    static void splitStrip(const QString &file, int size);
    static QImage getSubImage(const QImage &img, const QRect &rect);
    static QList<QImage> splitCubemap(const QString &file, bool save = false);

private:
    GLuint texID;
};

//-------------------------------------------------------------------

class Skybox {
public:
    Skybox() : vertexBuffer(0), indexBuffer(0) {}
    ~Skybox();

    QImage setTexture(const QList<QImage> &cubemap);
    QImage setTexture(const QString &cubemap);

    void init(GLuint shaderProgram, GLuint texSampler, GLuint mvp, GLuint wm);
    void render(const QMatrix4x4 &mvp, bool wireframe = false);

private:
    GLuint shaderProgramID, texSamplerID, mvpID, wmID;
    GLuint vertexBuffer, indexBuffer;

    CubemapTexture tex;
};

//-------------------------------------------------------------------

class Terrain {
public:
    Terrain() : vertexBuffer(0), normalBuffer(0), texID(0), normalTexID(0) {}
    ~Terrain();

    bool ready() const {
        return !vertexCoords.empty();
    }

    void init(GLuint shaderProgram, GLuint texSampler, GLuint mvp, GLuint wm, GLuint tm, GLuint uc);
    void generatePlane(float planeZSize, float planeXSize, float cellSize);
    void generateHeightMap(float persistence, float frequency, float amplitude, int octaves);
    void bindBuffer();

    void setTexture(const QImage &img, bool terrain = true);
    void render(const QMatrix4x4 &mvp, bool wireframe = false, int texMode = 0, float contrast = 1.0);

private:
    void computeNormals();
    inline void incGridNormal(QVector<QPair<QVector3D, int> > &norms, const QVector3D &v, int x, int z);
    inline void incGridNormal(QVector<QPair<QVector3D, int> > &norms, const QVector3D &left, const QVector3D &right, int x, int z, bool isEven);
    inline QVector3D getVertex(int x, int y) const;
    inline QPolygon getXZTriangle(const QVector3D &v1, const QVector3D &v2, const QVector3D &v3);
    inline QColor colorFromNorm(const QVector3D &norm);

    GLuint shaderProgramID, mvpID, wmID, texSamplerID, texModeID, contrastID;
    GLuint vertexBuffer, texCoordBuffer, indexBuffer, indexBufferSize, normalBuffer;
    GLuint texID, normalTexID;

    float gridSize;
    int vW, vL;
    QVector<float> vertexCoords, vertexNormals;
};

//-------------------------------------------------------------------

class PerlinNoise {
public:
    PerlinNoise();
    PerlinNoise(double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed);

    double getHeight(double x, double y) const;
    void init(double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed);

private:
    double total(double i, double j) const;
    double getValue(double x, double y) const;
    double interpolate(double x, double y, double a) const;
    double noise(int x, int y) const;

    double persistence, frequency, amplitude;
    int octaves, randomseed;
};

//-------------------------------------------------------------------

class CameraFrustum : public QObject {
    Q_OBJECT

public:
    CameraFrustum();
    ~CameraFrustum();

    void setModel(const QString &model);
    void init(GLuint shaderProgram, GLuint mvp, GLuint wm, GLuint mc);
    void update(const QVector3D &cameraPos, const QVector3D &cameraDir, float far, float fov, float ratio);
    void render(const QMatrix4x4 &vp, const QVector3D &cameraPos, const QVector<int> &octs, float cubeSize = 0.0);

private slots:
    void setModelBuffer();
    void renderCube(const QMatrix4x4 &vp, const QVector3D &pos, float cubeSize, int i, const QVector<int> &octs);
    void renderCubePrecomp(bool wireframe = false);

private:
    QQuaternion rotationBetweenVectors(const QVector3D &start, const QVector3D &dest) const;

    GLuint shaderProgramID, mvpID, wmID, colorID;
    GLuint vertexBuffer, vertexBufferSize;
    GLuint cubeVertexBuffer, cubeIndexBuffer;

    OBJModel *mFrustum;
    QMatrix4x4 mModel;
};


#endif // TERRAIN_H
