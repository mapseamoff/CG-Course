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

QImage CubemapTexture::getSubImage(const QImage &img, const QRect &rect) {
    size_t offset = rect.x() * img.depth() / 8 + rect.y() * img.bytesPerLine();
    return QImage(img.bits() + offset, rect.width(), rect.height(), img.bytesPerLine(), img.format());
}

QList<QImage> CubemapTexture::splitCubemap(const QString &file, bool save) {
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

void CubemapTexture::splitStrip(const QString &file, int size) {
    QImage img(file);
    for(int i = 0; i < 6; ++i) {
        QImage simg = getSubImage(img, QRect(i * size, 0, size, size));
        simg.save(QString("img%1.png").arg(i));
    }
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
    glDeleteBuffers(1, &normalBuffer);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &texCoordBuffer);
    glDeleteTextures(1, &texID);
    glDeleteTextures(1, &normalTexID);
}

void Terrain::init(GLuint shaderProgram, GLuint texSampler, GLuint mvp, GLuint wm, GLuint tm, GLuint uc) {
    shaderProgramID = shaderProgram;
    texSamplerID = texSampler;
    mvpID = mvp;
    wmID = wm;
    texModeID = tm;
    contrastID = uc;
}

void Terrain::generatePlane(float planeZSize, float planeXSize, float cellSize) {
    gridSize = cellSize;

    if(vertexBuffer != 0) {
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
        glDeleteBuffers(1, &texCoordBuffer);
        vertexBuffer = 0;
    }

    vL = planeZSize / cellSize + 1;
    vW = planeXSize / cellSize + 1;

    QVector<QVector3D> vcoords(vL * vW);
    QVector<QVector2D> tcoords(vL * vW);

    float halfW = ((float)vW - 1.0f) / 2.0f;
    float halfL = ((float)vL - 1.0f) / 2.0f;
    for(int z = 0; z < vL; ++z) {
        for(int x = 0; x < vW; ++x) {
            vcoords[z * vW + x] = QVector3D(((float)x - halfW) * cellSize, 0.0, ((float)z - halfL) * cellSize);
            tcoords[z * vW + x] = QVector2D((float)x / (vW - 1.0),  (float)z / (vL - 1.0));
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
//        vertexCoords[i] = generator.getHeight(vertexCoords[i - 1] / gridSize, vertexCoords[i + 1] / gridSize);
        vertexCoords[i] = generator.getHeight(vertexCoords[i - 1], vertexCoords[i + 1]);
    }
    computeNormals();
}

#include <QPainter>
#include <QColor>
#include <qmath.h>

inline QVector3D Terrain::getVertex(int x, int y) const {
    const float *i = &vertexCoords[3 * (y * vW + x)];
    return QVector3D(*i, *(i + 1), *(i + 2));
}

inline QPolygon Terrain::getXZTriangle(const QVector3D &v1, const QVector3D &v2, const QVector3D &v3) {
    int xShift = (vW - 1) / 2 * gridSize;
    int zShift = (vW - 1) / 2 * gridSize;
    QVector<QPoint> pts;
    pts.append(QPoint(v1.x() + xShift, v1.z() + zShift));
    pts.append(QPoint(v2.x() + xShift, v2.z() + zShift));
    pts.append(QPoint(v3.x() + xShift, v3.z() + zShift));
    return QPolygon(pts);
}

inline QColor Terrain::colorFromNorm(const QVector3D &norm) {
    return QColor(qAbs(norm.x()) * 255, qAbs(norm.y()) * 255, qAbs(norm.z()) * 255);
}

inline void Terrain::incGridNormal(QVector<QPair<QVector3D, int> > &norms, const QVector3D &v, int x, int z) {
    QPair<QVector3D, int> &n = norms[z * vW + x];
    n.first += v;
    n.second++;
}

inline void Terrain::incGridNormal(QVector<QPair<QVector3D, int> > &norms, const QVector3D &left, const QVector3D &right, int x, int z, bool isEven) {
    if(isEven) {
        incGridNormal(norms, left, x, z);
        incGridNormal(norms, left, x + 1, z);
        incGridNormal(norms, left, x, z + 1);
        incGridNormal(norms, right, x + 1, z + 1);
        incGridNormal(norms, right, x + 1, z);
        incGridNormal(norms, right, x, z + 1);
    } else {
        incGridNormal(norms, left, x, z);
        incGridNormal(norms, left, x, z + 1);
        incGridNormal(norms, left, x + 1, z + 1);
        incGridNormal(norms, right, x, z);
        incGridNormal(norms, right, x + 1, z);
        incGridNormal(norms, right, x + 1, z + 1);
    }
}

void Terrain::computeNormals() {
    QImage img((int)gridSize * (vW - 1), (int)gridSize * (vL - 1), QImage::Format_RGB888);
    QPainter p(&img);
    p.setPen(Qt::NoPen);

    QVector<QPair<QVector3D, int> > gridNormals(vertexCoords.size() / 3);
    for(int z = 0; z < vL - 1; ++z) {
        for(int x = 0; x < vW - 1; ++x) {
            QVector3D v1 = getVertex(x, z);
            QVector3D v2 = getVertex(x + 1, z);
            QVector3D v3 = getVertex(x, z + 1);
            QVector3D v4 = getVertex(x + 1, z + 1);
            QVector3D leftTriangle, rightTriangle;
            if(z % 2 == 0){
                leftTriangle = QVector3D::normal(v2 - v1, v3 - v1);
                rightTriangle = QVector3D::normal(v2 - v4, v3 - v4);

                p.setBrush(QBrush(colorFromNorm(leftTriangle)));
                p.drawPolygon(getXZTriangle(v1, v2, v3));
                p.setBrush(QBrush(colorFromNorm(rightTriangle)));
                p.drawPolygon(getXZTriangle(v4, v2, v3));
            } else {
                leftTriangle = QVector3D::normal(v1 - v3, v4 - v3);
                rightTriangle = QVector3D::normal(v1 - v2, v4 - v2);

                p.setBrush(QBrush(colorFromNorm(leftTriangle)));
                p.drawPolygon(getXZTriangle(v1, v3, v4));
                p.setBrush(QBrush(colorFromNorm(rightTriangle)));
                p.drawPolygon(getXZTriangle(v1, v2, v4));
            }
            if(leftTriangle.y() < 0) leftTriangle *= -1;
            if(rightTriangle.y() < 0) rightTriangle *= -1;
            incGridNormal(gridNormals, leftTriangle, rightTriangle, x, z, z % 2 == 0);
        }
    }
    p.end();

    vertexNormals.clear();
    for(QVector<QPair<QVector3D, int> >::Iterator i = gridNormals.begin(); i != gridNormals.end(); ++i) {
//        QVector3D n = i->first / (double)i->second;
        QVector3D n = i->first.normalized();
        vertexNormals.append(n.x());
        vertexNormals.append(n.y());
        vertexNormals.append(n.z());
    }

    setTexture(img, false);
//    img.save("norm.png");
}

void Terrain::bindBuffer() {
    if(vertexBuffer != 0) glDeleteBuffers(1, &vertexBuffer);
    if(normalBuffer != 0) glDeleteBuffers(1, &normalBuffer);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCoords.size() * sizeof(float), &vertexCoords[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexNormals.size() * sizeof(float), &vertexNormals[0], GL_STATIC_DRAW);
}

void Terrain::setTexture(const QImage &img, bool terrain) {
    GLuint tex = terrain ? texID : normalTexID;
    if(tex != 0) glDeleteTextures(1, &tex);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if(terrain) texID = tex;
    else normalTexID = tex;
}

void Terrain::render(const QMatrix4x4 &mvp, bool wireframe, int texMode, float contrast) {
    if(vertexBuffer == 0) return;

    if(wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(shaderProgramID);
    glActiveTexture(GL_TEXTURE0);
    if(texMode == 0) glBindTexture(GL_TEXTURE_2D, texID);
    else glBindTexture(GL_TEXTURE_2D, normalTexID);
    glUniform1i(texSamplerID, 0);
    setUniformMatrix(glUniformMatrix4fv, mvpID, mvp, 4, 4);
    glUniform1i(wmID, wireframe ? 1 : 0);
    glUniform1i(texModeID, texMode);
    glUniform1f(contrastID, contrast);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    if(wireframe) glEnable(GL_POLYGON_OFFSET_FILL);
    glDrawElements(GL_TRIANGLE_STRIP, indexBufferSize, GL_UNSIGNED_INT, 0);
    if(wireframe) glDisable(GL_POLYGON_OFFSET_FILL);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

//===========================================================================================

CameraFrustum::CameraFrustum() : QObject(), vertexBuffer(0), vertexBufferSize(0), mFrustum(new OBJModel(this)) {
    connect(mFrustum, SIGNAL(loadStatus(bool)), this, SLOT(setModelBuffer()));
}

CameraFrustum::~CameraFrustum() {
    mFrustum->deleteLater();
    glDeleteBuffers(1, &vertexBuffer);
}

void CameraFrustum::setModel(const QString &model) {
    mFrustum->loadModel(model);
}

void CameraFrustum::setModelBuffer() {
    if(vertexBuffer != 0) glDeleteBuffers(1, &vertexBuffer);

    std::vector<OBJVec3> vs;
    for(std::vector<OBJFace>::iterator fi = mFrustum->faces.begin(); fi != mFrustum->faces.end(); ++fi) {
        for(OBJFace::iterator fii = fi->begin(); fii != fi->end(); ++fii) {
            vs.push_back(mFrustum->verts[fii->v - 1]);
        }
    }

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(OBJVec3), &vs[0], GL_STATIC_DRAW);
    vertexBufferSize = vs.size();
}

QQuaternion CameraFrustum::rotationBetweenVectors(const QVector3D &start, const QVector3D &dest) const {
    QVector3D _start = start.normalized();
    QVector3D _dest = dest.normalized();

    float cosTheta = QVector3D::dotProduct(_start, _dest);
    if(cosTheta < -1 + 0.001) {
        QVector3D rotationAxis = QVector3D::crossProduct(QVector3D(0, 0, 1), _start);
        if (rotationAxis.lengthSquared() < 0.01 ) {
            rotationAxis = QVector3D::crossProduct(QVector3D(1, 0, 0), _start);
        }
        return QQuaternion::fromAxisAndAngle(rotationAxis.normalized(), 180.0);
    }

    QVector3D rotationAxis = QVector3D::crossProduct(_start, _dest);

    float s = sqrt((1 + cosTheta) * 2);
    float invs = 1 / s;

    return QQuaternion(s * 0.5f, rotationAxis.x() * invs, rotationAxis.y() * invs, rotationAxis.z() * invs);
}

void CameraFrustum::update(const QVector3D &cameraPos, const QVector3D &cameraDir, const QVector3D &cameraRight,
                           float far, float fov, float ratio) {
    mModel.setToIdentity();

    QVector3D desiredUp = QVector3D::crossProduct(cameraRight, cameraDir);
    if(qAbs(QVector3D::dotProduct(desiredUp, QVector3D(0, 1, 0))) > 1e-6) {
        QQuaternion rot1 = rotationBetweenVectors(QVector3D(0, 0, -1), QVector3D(cameraDir.x(), 0, cameraDir.z()));
        QVector3D newUp = rot1.rotatedVector(QVector3D(0, 1, 0));
        QQuaternion rot2 = rotationBetweenVectors(newUp, desiredUp);
        mModel.rotate(rot2 * rot1);
    } else {
        QQuaternion rot1 = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), (cameraDir.y() > 0 ? 1 : -1) * 90.0);
        QQuaternion rot2 = rotationBetweenVectors(QVector3D(1, 0, 0), cameraRight);
        mModel.rotate(rot2 * rot1);
    }

    QVector4D realPos = cameraPos.toVector4D();
    realPos.setW(1.0);
    mModel.setColumn(3, realPos);
    double hs = 2 * tan(fov / 2) * far;
    double ws = hs * ratio;
    mModel.scale(ws, hs, far);
}

void CameraFrustum::init(GLuint shaderProgram, GLuint mvp, GLuint wm, GLuint mc) {
    shaderProgramID = shaderProgram;
    mvpID = mvp;
    wmID = wm;
    colorID = mc;

    float vertices[24] = {
        -0.5,	-0.5,	-0.5,
        0.5,	-0.5,	-0.5,
        -0.5,	0.5,	-0.5,
        0.5,	0.5,	-0.5,
        -0.5,	-0.5,	0.5,
        0.5,	-0.5,	0.5,
        -0.5,	0.5,	0.5,
        0.5,	0.5,	0.5
    };

    glGenBuffers(1, &cubeVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned char indices[24] = {
        1,			5,			7,			3,	// positive x
        2,			0,			4,			6,	// negative x
        4,			5,			7,			6,	// positive y
        0,			1,			3,			2,	// negative y
        0,			1,			5,			4,	// positive z
        3,			2,			6,			7	// negative z
    };

    glGenBuffers(1, &cubeIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void CameraFrustum::render(const QMatrix4x4 &vp, const QVector3D &cameraPos, int octs, float cubeSize) {
    if(vertexBuffer == 0) return;

    QMatrix4x4 mvp = vp * mModel;

    glUseProgram(shaderProgramID);
    setUniformMatrix(glUniformMatrix4fv, mvpID, mvp, 4, 4);
    glUniform3f(colorID, 1.0, 1.0, 1.0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUniform1i(wmID, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLES, 0, vertexBufferSize);
    glDisableVertexAttribArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUniform1i(wmID, 1);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glDrawArrays(GL_TRIANGLES, 0, vertexBufferSize);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisableVertexAttribArray(0);

    float osz = cubeSize / 2.0;
    float cs = osz / 2.0 + 0.1;
    renderCube(vp, cameraPos + QVector3D(-cs, cs, -cs), osz, octs & (1 << 0));
    renderCube(vp, cameraPos + QVector3D(cs, cs, -cs),  osz, octs & (1 << 1));
    renderCube(vp, cameraPos + QVector3D(-cs, cs, cs),  osz, octs & (1 << 2));
    renderCube(vp, cameraPos + QVector3D(cs, cs, cs),   osz, octs & (1 << 3));
    renderCube(vp, cameraPos + QVector3D(-cs, -cs, -cs),osz, octs & (1 << 4));
    renderCube(vp, cameraPos + QVector3D(cs, -cs, -cs), osz, octs & (1 << 5));
    renderCube(vp, cameraPos + QVector3D(-cs, -cs, cs), osz, octs & (1 << 6));
    renderCube(vp, cameraPos + QVector3D(cs, -cs, cs),  osz, octs & (1 << 7));
}

void CameraFrustum::renderCube(const QMatrix4x4 &vp, const QVector3D &pos, float cubeSize, bool ints) {
    QMatrix4x4 mm;
    mm.setToIdentity();
    mm.translate(pos);
    mm.scale(cubeSize);
    QMatrix4x4 mvp = vp * mm;

    glUseProgram(shaderProgramID);
    setUniformMatrix(glUniformMatrix4fv, mvpID, mvp, 4, 4);
    if(ints) glUniform3f(colorID, 0.0, 1.0, 0.0);
    else glUniform3f(colorID, 1.0, 0.5, 0.0);
    renderCubePrecomp(false);
    renderCubePrecomp(true);
}

void CameraFrustum::renderCubePrecomp(bool wireframe) {
    if(wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUniform1i(wmID, wireframe ? 1 : 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    if(wireframe) glEnable(GL_POLYGON_OFFSET_FILL);
    glDrawElements(GL_QUADS, 6 * 4, GL_UNSIGNED_BYTE, 0);
    if(wireframe) glDisable(GL_POLYGON_OFFSET_FILL);
    glDisableVertexAttribArray(0);
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
