#ifndef FRUSTUMUTILS_H
#define FRUSTUMUTILS_H

#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QVector>

class FrustumUtils {
public:
    static QVector<int> getIntersections(const QMatrix4x4 &vp, const QVector3D &camPos, float cubeSize);
    static int getIntersectionsAsInt(const QMatrix4x4 &vp, const QVector3D &camPos, float cubeSize);

private:
    static QVector<QVector4D> extractPlanes(const QMatrix4x4 &vp);
    static bool intersects(const QVector<QVector4D> &pl, const QVector3D &pos, float halfSize);
};

#endif // FRUSTUMUTILS_H
