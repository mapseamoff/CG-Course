#include "objmodel.h"

#include <QFile>
#include <QTextStream>

#define FDM_VTN 1
#define FDM_VT  2
#define FDM_VN  3
#define FDM_V   4

/**************************************************************************************/

OBJModel::OBJModel(QObject *parent) : QObject(parent) {
    loader = new OBJModelLoadingThread(faces, verts, texs, norms, texture, this);
    connect(loader, SIGNAL(loadProgress(int)), this, SLOT(progressSignal(int)));
    connect(loader, SIGNAL(finished()), this, SLOT(loadingFinished()));
}

void OBJModel::loadModel(const QString &filePath, const QString &texPath) {
    loader->setFileName(filePath, texPath);
    loader->start();
}

void OBJModel::stopLoading() {
    loader->stopThread = true;
}

void OBJModel::progressSignal(int val) {
    emit loadProgress(val);
}

void OBJModel::loadingFinished() {
    emit loadStatus(loader->modelStatus);
}

/**************************************************************************************/

OBJModelLoadingThread::OBJModelLoadingThread(FaceVector &f, VertexVector &v, VertexVector &t, VertexVector &n, QImage &tex, QObject *parent)
    : QThread(parent), modelStatus(false), stopThread(false), modelError(""), filePath(""), texPath(""), faces(f), verts(v), texs(t), norms(n), tex(tex) {
}

void OBJModelLoadingThread::setFileName(const QString &fp, const QString &tp) {
    filePath = fp;
    texPath = tp;
}

void OBJModelLoadingThread::run() {
    stopThread = false;
    modelError = "";
    QFile fileIn(filePath);
    if(!fileIn.open(QFile::ReadOnly)) {
        modelStatus = false;
        modelError = "unable to open model file";
        return;
    }
    QTextStream ts(&fileIn);

    faces.clear();
    verts.clear();
    texs.clear();
    norms.clear();

    emit loadProgress(0);

    qint64 fileSize = fileIn.size();
    OBJVec3 v;
    QString cmd;
    for(size_t lineCounter = 1; !ts.atEnd(); ++lineCounter) {
        if(stopThread) {
            modelStatus = false;
            return;
        }
        ts >> cmd;
        if(cmd == "v") {
            ts >> v.x >> v.y >> v.z;
            verts.push_back(v);
        } else if(cmd == "vt") {
            ts >> v.x >> v.y;
            v.z = 0;
            texs.push_back(v);
        } else if(cmd == "vn") {
            ts >> v.x >> v.y >> v.z;
            norms.push_back(v);
        } else if(cmd == "f") {
            ts.skipWhiteSpace();
            OBJFace f;
            QString faceDescr = ts.readLine();
            size_t matchMethod = 0;
            for(int cpos = 0, opos = 0; cpos != -1; opos = cpos + 1) {
                FaceIndex i;
                cpos = faceDescr.indexOf(' ', opos);
                if(!matchFaceDescr(faceDescr.mid(opos, cpos == -1 ? cpos : cpos - opos), matchMethod, i)) {
                    modelError += QString("unable to parse face at line %1\n").arg(lineCounter);
                    modelStatus = false;
                    return;
                }
                if(i.v > verts.size() || i.n > norms.size() || i.t > texs.size()) {
                    modelError = QString("index out of bound at line %1\n").arg(lineCounter);
                    modelStatus = false;
                    return;
                }
                f.push_back(i);
//                if(i.v != 0) f.vs.push_back(vertices[i.v - 1]);
//                if(i.t != 0) f.ts.push_back(texs[i.t - 1]);
//                if(i.n != 0) f.ns.push_back(norms[i.n - 1]);
            }
            if(f.size() != 3) {
                modelError += QString("only triangles supported\n");
                modelStatus = false;
                return;
            }
            faces.push_back(f);
            continue;
        } else if(cmd.at(0) != '#') {
            modelError += QString("Warning: unsupported command '%1' at line %2\n").arg(cmd).arg(lineCounter);
        }
        ts.readLine();
        int lp = 100 * fileIn.pos() / fileSize;
        emit loadProgress(lp < 100 ? lp : 99);
    }
    fileIn.close();
    if(!texPath.isEmpty()) {
        tex = QImage(texPath).convertToFormat(QImage::Format_RGB888);
        if(tex.isNull()) {
            modelError += QString("Unable to load texture");
            modelStatus = false;
            return;
        }
    }
    emit loadProgress(100);
    modelStatus = true;
}

bool OBJModelLoadingThread::matchFaceDescr(QString str, size_t &method, FaceIndex &out) const {
    QTextStream s(&str, QIODevice::ReadOnly);

    switch(method) {
    case FDM_V:
        s >> out.v;
        break;
    case FDM_VTN:
        s >> out.v; s.read(1);
        s >> out.t; s.read(1);
        s >> out.n;
        break;
    case FDM_VN:
        s >> out.v; s.read(2);
        s >> out.n;
        break;
    case FDM_VT:
        s >> out.v; s.read(1);
        s >> out.t;
        break;
    default:
        if(str.indexOf("//") != -1) {
            method = FDM_VN;
        } else {
            switch(str.count('/')) {
            case 0: method = FDM_V; break;
            case 1: method = FDM_VT; break;
            case 2: method = FDM_VTN; break;
            default: return false;
            }
        }
        return matchFaceDescr(str, method, out);
    }
    return true;
}
