#include "terrain.h"

#include <QVector2D>

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

//===========================================================================================

CubemapTexture::~CubemapTexture() {
    glDeleteTextures(1, &texID);
}

QImage CubemapTexture::load(const QString &posX, const QString &negX,
          const QString &posY, const QString &negY,
          const QString &posZ, const QString &negZ) {
    if(texID != 0) glDeleteTextures(1, &texID);

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
        if(tex.isNull()) return QImage();
        glTexImage2D(i.key(), 0, GL_RGB, tex.width(), tex.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, tex.bits());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return QImage();
}

QImage CubemapTexture::load(const QList<QImage> &imgs) {
    if(texID != 0) glDeleteTextures(1, &texID);

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

    return imgs.at(3);
}

QImage CubemapTexture::load(const QString &cubemap) {
    return load(splitCubemap(cubemap));
}

QImage CubemapTexture::getSubImage(const QImage &img, const QRect &rect) const {
    size_t offset = rect.x() * img.depth() / 8 + rect.y() * img.bytesPerLine();
    return QImage(img.bits() + offset, rect.width(), rect.height(), img.bytesPerLine(), img.format());
}

QList<QImage> CubemapTexture::splitCubemap(const QString &file, bool save) const {
    QImage img(file);
    int rw = 0;
    int rh = 0;
    for(int i = 0; i < img.width(); ++i) {
        if(rw == 0 && img.pixel(i, 0) != qRgb(255,255,255)) rw = i;
        if(rh == 0 && img.pixel(0, i) != qRgb(255,255,255)) rh = i;
        if(rw != 0 && rh != 0) break;
    }

    QImage posY = getSubImage(img, QRect(rw, 0, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage negY = getSubImage(img, QRect(rw, 2*rh, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage negX = getSubImage(img, QRect(0, rh, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage posZ = getSubImage(img, QRect(rw, rh, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage posX = getSubImage(img, QRect(2*rw, rh, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage negZ = getSubImage(img, QRect(3*rw, rh, rw, rh)).convertToFormat(QImage::Format_RGB888);

    if(save) {
        posX.save("posX.png"); negX.save("negX.png");
        posY.save("posY.png"); negY.save("negY.png");
        posZ.save("posZ.png"); negZ.save("negZ.png");
    }

    QList<QImage> res;
    res.append(posX); res.append(negX);
    res.append(posY); res.append(negY);
    res.append(posZ); res.append(negZ);
    return res;
}

//===========================================================================================

Skybox::~Skybox() {
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &indexBuffer);
}

QImage Skybox::setTexture(const QList<QImage> &cubemap) {
    return tex.load(cubemap);
}

QImage Skybox::setTexture(const QString &cubemap) {
    return tex.load(cubemap);
}

void Skybox::init(GLuint shaderProgram, GLuint texSampler, GLuint mvp, GLuint wm) {
    shaderProgramID = shaderProgram;
    texSamplerID = texSampler;
    mvpID = mvp;
    wmID = wm;

    float vertices[24] = {
        -1.0,	-1.0,	-1.0,
        1.0,	-1.0,	-1.0,
        -1.0,	1.0,	-1.0,
        1.0,	1.0,	-1.0,
        -1.0,	-1.0,	1.0,
        1.0,	-1.0,	1.0,
        -1.0,	1.0,	1.0,
        1.0,	1.0,	1.0
    };

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned char indices[24] = {
        1,			5,			7,			3,	// positive x
        2,			0,			4,			6,	// negative x
        4,			5,			7,			6,	// positive y
        0,			1,			3,			2,	// negative y
        0,			1,			5,			4,	// positive z
        3,			2,			6,			7	// negative z
    };

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Skybox::render(const QMatrix4x4 &mvp, bool wireframe) {
    GLboolean oldDepthMaskMode;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthMaskMode);
    glDepthMask(GL_FALSE);

    if(wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(shaderProgramID);
    setUniformMatrix(glUniformMatrix4fv, mvpID, mvp, 4, 4);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex.getTexID());
    glUniform1i(texSamplerID, 0);
    glUniform1i(wmID, wireframe ? 1 : 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    if(wireframe) glEnable(GL_POLYGON_OFFSET_FILL);
    glDrawElements(GL_QUADS, 6 * 4, GL_UNSIGNED_BYTE, 0);
    if(wireframe) glDisable(GL_POLYGON_OFFSET_FILL);
    glDisableVertexAttribArray(0);

    glDepthMask(oldDepthMaskMode);
}

//===========================================================================================

Terrain::~Terrain() {
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &texCoordBuffer);
    glDeleteTextures(1, &texID);
}

void Terrain::init(GLuint shaderProgram, GLuint texSampler, GLuint mvp, GLuint wm) {
    shaderProgramID = shaderProgram;
    texSamplerID = texSampler;
    mvpID = mvp;
    wmID = wm;
}

void Terrain::generatePlane(float planeZSize, float planeXSize, float cellSize) {
    if(vertexBuffer != 0) {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
        glDeleteBuffers(1, &texCoordBuffer);
        vertexBuffer = 0;
    }

    int vL = planeZSize / cellSize + 1;
    int vW = planeXSize / cellSize + 1;

    QVector<QVector3D> vcoords(vL * vW);
    QVector<QVector2D> tcoords(vL * vW);

    float halfW = ((float)vW - 1.0f) / 2.0f;
    float halfL = ((float)vL - 1.0f) / 2.0f;
    for(int z = 0; z < vL; ++z) {
        for(int x = 0; x < vW; ++x) {
            vcoords[z * vW + x] = QVector3D(((float)x - halfW) * cellSize, 0.0, ((float)z - halfL) * cellSize);
            tcoords[z * vW] = QVector2D((float)x / (vW - 1.0),  (float)z / (vL - 1.0));
        }
    }

    vertexCoords.clear();
    QVector<float> texCoords;
    for(QVector<QVector3D>::Iterator it = vcoords.begin(); it != vcoords.end(); ++it) {
        vertexCoords.push_back(it->x());
        vertexCoords.push_back(it->y());
        vertexCoords.push_back(it->z());
    }
    for(QVector<QVector2D>::Iterator it = tcoords.begin(); it != tcoords.end(); ++it) {
        texCoords.push_back(it->x());
        texCoords.push_back(it->y());
    }

    indexBufferSize = (vW * 2) * (vL - 1) + (vL - 2);
    QVector<unsigned int> indices(indexBufferSize);

    int index = 0, x = 0;
    for(int z = 0; z < vL - 1; z++) {
        if(z % 2 == 0) {
            // Even rows - left to right
            for(x = 0; x < vW; x++) {
                indices[index++] = x + (z * vW);
                indices[index++] = x + (z * vW) + vW;
            }
            // Insert degenerate vertex if this isn't the last row
            if(z != vL - 2) indices[index++] = --x + (z * vW);
        } else {
            // Odd rows - right to left.
            for(x = vW - 1; x >= 0; x--) {
                indices[index++] = x + (z * vW);
                indices[index++] = x + (z * vW) + vW;
            }
            if(z != vL - 2) indices[index++] = ++x + (z * vW);
        }
    }

    glGenBuffers(1, &texCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), &texCoords[0], GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
}

void Terrain::generateHeightMap(float persistence, float frequency, float amplitude, int octaves) {
    PerlinNoise generator(persistence, frequency, amplitude, octaves, qrand());
    for(int i = 1; i < vertexCoords.size(); i += 3) {
        vertexCoords[i] = generator.getHeight(vertexCoords[i - 1], vertexCoords[i + 1]);
    }
}

void Terrain::bindBuffer() {
    if(vertexBuffer != 0) glDeleteBuffers(1, &vertexBuffer);
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.size() * sizeof(float), &vertexCoords[0], GL_STATIC_DRAW);
}

void Terrain::setTexture(const QImage &img) {
    if(texID != 0) glDeleteTextures(1, &texID);
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void Terrain::render(const QMatrix4x4 &mvp, bool wireframe) {
    if(vertexBuffer == 0) return;

    if(wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(shaderProgramID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(texSamplerID, 0);
    setUniformMatrix(glUniformMatrix4fv, mvpID, mvp, 4, 4);
    glUniform1i(wmID, wireframe ? 1 : 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    if(wireframe) glEnable(GL_POLYGON_OFFSET_FILL);
    glDrawElements(GL_TRIANGLE_STRIP, indexBufferSize, GL_UNSIGNED_INT, 0);
    if(wireframe) glDisable(GL_POLYGON_OFFSET_FILL);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

//===========================================================================================

PerlinNoise::PerlinNoise() : persistence(0), frequency(0), amplitude(0), octaves(0), randomseed(0) {}

PerlinNoise::PerlinNoise(double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed) :
    persistence(_persistence), frequency(_frequency), amplitude(_amplitude), octaves(_octaves), randomseed(2 + _randomseed * _randomseed) {}

void PerlinNoise::init(double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed) {
    persistence = _persistence;
    frequency = _frequency;
    amplitude  = _amplitude;
    octaves = _octaves;
    randomseed = 2 + _randomseed * _randomseed;
}

double PerlinNoise::getHeight(double x, double y) const {
    return amplitude * total(x, y);
}

double PerlinNoise::total(double i, double j) const {
    //properties of one octave (changing each loop)
    double t = 0.0f;
    double _amplitude = 1;
    double freq = frequency;

    for(int k = 0; k < octaves; ++k) {
        t += getValue(j * freq + randomseed, i * freq + randomseed) * _amplitude;
        _amplitude *= persistence;
        freq *= 2;
    }

    return t;
}

double PerlinNoise::getValue(double x, double y) const {
    int Xint = (int)x;
    int Yint = (int)y;
    double Xfrac = x - Xint;
    double Yfrac = y - Yint;

    //noise values
    double n01 = noise(Xint-1, Yint-1);
    double n02 = noise(Xint+1, Yint-1);
    double n03 = noise(Xint-1, Yint+1);
    double n04 = noise(Xint+1, Yint+1);
    double n05 = noise(Xint-1, Yint);
    double n06 = noise(Xint+1, Yint);
    double n07 = noise(Xint, Yint-1);
    double n08 = noise(Xint, Yint+1);
    double n09 = noise(Xint, Yint);

    double n12 = noise(Xint+2, Yint-1);
    double n14 = noise(Xint+2, Yint+1);
    double n16 = noise(Xint+2, Yint);

    double n23 = noise(Xint-1, Yint+2);
    double n24 = noise(Xint+1, Yint+2);
    double n28 = noise(Xint, Yint+2);

    double n34 = noise(Xint+2, Yint+2);

    //find the noise values of the four corners
    double x0y0 = 0.0625*(n01+n02+n03+n04) + 0.125*(n05+n06+n07+n08) + 0.25*(n09);
    double x1y0 = 0.0625*(n07+n12+n08+n14) + 0.125*(n09+n16+n02+n04) + 0.25*(n06);
    double x0y1 = 0.0625*(n05+n06+n23+n24) + 0.125*(n03+n04+n09+n28) + 0.25*(n08);
    double x1y1 = 0.0625*(n09+n16+n28+n34) + 0.125*(n08+n14+n06+n24) + 0.25*(n04);

    //interpolate between those values according to the x and y fractions
    double v1 = interpolate(x0y0, x1y0, Xfrac); //interpolate in x direction (y)
    double v2 = interpolate(x0y1, x1y1, Xfrac); //interpolate in x direction (y+1)
    double fin = interpolate(v1, v2, Yfrac);  //interpolate in y direction

    return fin;
}

double PerlinNoise::interpolate(double x, double y, double a) const {
    double negA = 1.0 - a;
    double negASqr = negA * negA;
    double fac1 = 3.0 * (negASqr) - 2.0 * (negASqr * negA);
    double aSqr = a * a;
    double fac2 = 3.0 * aSqr - 2.0 * (aSqr * a);
    return x * fac1 + y * fac2;
}

double PerlinNoise::noise(int x, int y) const {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    return 1.0 - double(t) * 0.931322574615478515625e-9;/// 1073741824.0);
}
