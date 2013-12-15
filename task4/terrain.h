#ifndef TERRAIN_H
#define TERRAIN_H

#include <GL/glew.h>

#include <QList>
#include <QImage>
#include <QMatrix4x4>

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

private:
    QImage getSubImage(const QImage &img, const QRect &rect) const;
    QList<QImage> splitCubemap(const QString &file, bool save = false) const;

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
    Terrain() : vertexBuffer(0), texID(0) {}
    ~Terrain();

    void init(GLuint shaderProgram, GLuint texSampler, GLuint mvp, GLuint wm);
    void generatePlane(float planeZSize, float planeXSize, float cellSize);
    void generateHeightMap(float persistence, float frequency, float amplitude, int octaves);
    void bindBuffer();

    void setTexture(const QImage &img);
    void render(const QMatrix4x4 &mvp, bool wireframe = false);

private:
    GLuint shaderProgramID, mvpID, wmID, texSamplerID;
    GLuint vertexBuffer, texCoordBuffer, indexBuffer, indexBufferSize;
    GLuint texID;

    QVector<float> vertexCoords;
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


#endif // TERRAIN_H
