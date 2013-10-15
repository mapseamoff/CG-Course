#ifndef OBJMODEL_H
#define OBJMODEL_H

#include <GL/glew.h>

#include <QObject>
#include <QThread>

#include <vector>
#include <string>

struct OBJVec3 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
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
    OBJModelLoadingThread(FaceVector &f, VertexVector &v, VertexVector &t, VertexVector &n, QObject *parent = 0);
    void setFileName(const std::string &fp);

    bool modelStatus;
    volatile bool stopThread;
    std::string modelError;

signals:
    void loadProgress(int val);

private:
    void run();

private:
    std::string filePath;
    FaceVector &faces;
    VertexVector &verts, &texs, &norms;

    bool matchFaceDescr(const std::string &str, size_t &method, FaceIndex &out) const;
};

//----------------------------------------------------------------------------------------

class OBJModel : public QObject {
    Q_OBJECT

public:
    OBJModel(QObject *parent = 0);

    bool status() const { return loader->modelStatus; }
    void loadModel(const std::string &filePath);
    std::string modelError() const { return loader->modelError; }

    std::vector<OBJFace> faces;
    std::vector<OBJVec3> verts, texs, norms;

signals:
    void loadProgress(int val);
    void loadStatus(bool status);

public slots:
    void stopLoading();

private slots:
    void progressSignal(int val);
    void loadingFinished();

private:
    OBJModelLoadingThread *loader;
};

#endif // OBJMODEL_H
