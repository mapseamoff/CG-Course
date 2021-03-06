#ifndef OBJMODEL_H
#define OBJMODEL_H

#include <GL/glew.h>

#include <QObject>
#include <QThread>
#include <QImage>
#include <QVector3D>

#include <vector>
#include <string>

struct OBJVec3 {
    OBJVec3() : x(0.0), y(0.0), z(0.0) {}

    OBJVec3& operator+=(const OBJVec3 &other) {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;
        return *this;
    }

    OBJVec3& operator-=(const OBJVec3 &other) {
        this->x -= other.x;
        this->y -= other.y;
        this->z -= other.z;
        return *this;
    }

    OBJVec3& operator/=(const GLfloat &val) {
        this->x /= val;
        this->y /= val;
        this->z /= val;
        return *this;
    }

    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct OBJVec2 {
    OBJVec2(GLfloat u = 0.0, GLfloat v = 0.0) : u(u), v(v) {}
    GLfloat u;
    GLfloat v;
};

struct FaceIndex {
    FaceIndex(size_t v = 0, size_t t = 0, size_t n = 0) : v(v), t(t), n(n) {}
    size_t v;
    size_t t;
    size_t n;
};

typedef std::vector<FaceIndex> OBJFace;
typedef std::vector<OBJFace> FaceVector;
typedef std::vector<OBJVec3> VertexVector;

//----------------------------------------------------------------------------------------

class OBJModelLoadingThread : public QThread {
    Q_OBJECT

public:
    OBJModelLoadingThread(FaceVector &f, VertexVector &v, VertexVector &t, VertexVector &n, QImage &tex, QObject *parent = 0);
    void setFileName(const QString &fp, const QString &tp = "");

    bool modelStatus;
    volatile bool stopThread;
    QString modelError;

signals:
    void loadProgress(int val);

private:
    void run();

private:
    QString filePath, texPath;
    FaceVector &faces;
    VertexVector &verts, &texs, &norms;
    QImage &tex;

    bool matchFaceDescr(QString str, size_t &method, FaceIndex &out) const;
};

//----------------------------------------------------------------------------------------

class OBJModel : public QObject {
    Q_OBJECT

public:
    OBJModel(QObject *parent = 0);

    bool status() const { return loader->modelStatus; }
    void loadModel(const QString &filePath, const QString &texPath = "");
    QString modelError() const { return loader->modelError; }

    void moveToMassCenter();

    std::vector<OBJFace> faces;
    std::vector<OBJVec3> verts, texs, norms;
    QImage texture;
    OBJVec3 massCenter;

signals:
    void loadProgress(int val);
    void loadStatus(bool status);

public slots:
    void stopLoading();

private slots:
    void progressSignal(int val);
    void loadingFinished();

private:
    OBJVec3 calcMassCenter() const;

    OBJModelLoadingThread *loader;
};

#endif // OBJMODEL_H
