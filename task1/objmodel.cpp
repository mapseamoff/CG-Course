#include "objmodel.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <math.h>

#define BUF_SIZE 4096
#define MAX_COMMAND_LENGTH 128

#define FDM_VTN 1
#define FDM_VT  2
#define FDM_VN  3
#define FDM_V   4

static std::string int2string(int val) {
    std::ostringstream s;
    s << val;
    return s.str();
}

/**************************************************************************************/

OBJModel::OBJModel(QObject *parent) : QObject(parent) {
    loader = new OBJModelLoadingThread(faces, verts, texs, norms, this);
    connect(loader, SIGNAL(loadProgress(int)), this, SLOT(progressSignal(int)));
    connect(loader, SIGNAL(finished()), this, SLOT(loadingFinished()));
}

void OBJModel::loadModel(const std::string &filePath) {
    loader->setFileName(filePath);
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

OBJModelLoadingThread::OBJModelLoadingThread(FaceVector &f, VertexVector &v, VertexVector &t, VertexVector &n, QObject *parent)
    : QThread(parent), modelStatus(false), stopThread(false), modelError(""), filePath(""), faces(f), verts(v), texs(t), norms(n) {
}

void OBJModelLoadingThread::setFileName(const std::string &fp) {
    filePath = fp;
}

void OBJModelLoadingThread::run() {
    stopThread = false;
    modelError = "";
    std::ifstream fileIn(filePath.c_str());
    if(!fileIn.is_open()) {
        modelStatus = false;
        modelError ="unable to open model file";
        return;
    }

    faces.clear();
    verts.clear();
    texs.clear();
    norms.clear();

    char cmdbuf[MAX_COMMAND_LENGTH];
    char buf[BUF_SIZE];

    emit loadProgress(0);
    fileIn.seekg(0, std::ios_base::end);
    size_t fileSize = fileIn.tellg();
    fileIn.seekg(0, std::ios_base::beg);

    OBJVec3 v;
    for(size_t lineCounter = 1; !fileIn.eof(); ++lineCounter) {
        if(stopThread) {
            modelStatus = false;
            return;
        }
        fileIn.getline(cmdbuf, MAX_COMMAND_LENGTH, ' ');
        if(fileIn.eof()) break;
        if(fileIn.fail()) {
            modelError += std::string("unable to parse command at line ") + int2string(lineCounter) + "\n";
            modelStatus = false;
            return;
        }
        if(strcmp(cmdbuf, "v") == 0) {
            fileIn >> v.x >> v.y >> v.z;
            verts.push_back(v);
        } else if(strcmp(cmdbuf, "vt") == 0) {
            fileIn >> v.x >> v.y;
            v.z = 0;
            texs.push_back(v);
        } else if(strcmp(cmdbuf, "vn") == 0) {
            fileIn >> v.x >> v.y >> v.z;
            norms.push_back(v);
        } else if(strcmp(cmdbuf, "f") == 0) {
            OBJFace f;
            fileIn.getline(buf, BUF_SIZE);
            std::string faceDescr(buf);
            size_t matchMethod = 0;
            for(size_t cpos = 0, opos = 0; cpos != std::string::npos; opos = cpos + 1) {
                FaceIndex i;
                cpos = faceDescr.find(' ', opos);
                if(!matchFaceDescr(faceDescr.substr(opos, cpos == std::string::npos ? cpos : cpos - opos), matchMethod, i)) {
                    modelError += std::string("unable to parse face at line ") + int2string(lineCounter) + "\n";
                    modelStatus = false;
                    return;
                }
                if(i.v > verts.size() || i.n > norms.size() || i.t > texs.size()) {
                    modelError = std::string("index out of bound at line ") + int2string(lineCounter) + "\n";
                    modelStatus = false;
                    return;
                }
                f.push_back(i);
//                if(i.v != 0) f.vs.push_back(vertices[i.v - 1]);
//                if(i.t != 0) f.ts.push_back(texs[i.t - 1]);
//                if(i.n != 0) f.ns.push_back(norms[i.n - 1]);
            }
            if(f.size() != 3) {
                modelError += std::string("only triangles supported\n");
                modelStatus = false;
                return;
            }
            faces.push_back(f);
            continue;
        } else if(cmdbuf[0] != '#') {
            modelError += "Warning: unsupported command \'" + std::string(cmdbuf) + "\' at line " + int2string(lineCounter) + "\n";
        }
        fileIn.getline(buf, BUF_SIZE);
        emit loadProgress(100 * fileIn.tellg() / fileSize);
    }
    fileIn.close();
    emit loadProgress(100);
    modelStatus = true;
}

bool OBJModelLoadingThread::matchFaceDescr(const std::string &str, size_t &method, FaceIndex &out) const {
    std::istringstream s(str);
    switch(method) {
    case FDM_V:
        s >> out.v;
        break;
    case FDM_VTN:
        s >> out.v; s.ignore();
        s >> out.t; s.ignore();
        s >> out.n;
        break;
    case FDM_VN:
        s >> out.v; s.ignore(); s.ignore();
        s >> out.n;
        break;
    case FDM_VT:
        s >> out.v; s.ignore();
        s >> out.t;
        break;
    default:
        if(str.find("//") != std::string::npos) {
            method = FDM_VN;
        } else {
            switch(std::count(str.begin(), str.end(), '/')) {
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
