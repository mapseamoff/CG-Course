#include "FrustumUtils.h"
#include <qmath.h>

struct FPlane {
    float a, b, c, d;
};

static void normalizeFPlane(FPlane &plane) {
    float mag = qSqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
    plane.a = plane.a / mag;
    plane.b = plane.b / mag;
    plane.c = plane.c / mag;
    plane.d = plane.d / mag;
}

QVector<QVector4D> FrustumUtils::extractPlanes(const QMatrix4x4 &vp) {
    FPlane p_planes[6];

    // Left clipping plane
    p_planes[0].a = vp(3, 0) + vp(0, 0);
    p_planes[0].b = vp(3, 1) + vp(0, 1);
    p_planes[0].c = vp(3, 2) + vp(0, 2);
    p_planes[0].d = vp(3, 3) + vp(0, 3);
    // Right clipping plane
    p_planes[1].a = vp(3, 0) - vp(0, 0);
    p_planes[1].b = vp(3, 1) - vp(0, 1);
    p_planes[1].c = vp(3, 2) - vp(0, 2);
    p_planes[1].d = vp(3, 3) - vp(0, 3);
    // Top clipping plane
    p_planes[2].a = vp(3, 0) - vp(1, 0);
    p_planes[2].b = vp(3, 1) - vp(1, 1);
    p_planes[2].c = vp(3, 2) - vp(1, 2);
    p_planes[2].d = vp(3, 3) - vp(1, 3);
    // Bottom clipping plane
    p_planes[3].a = vp(3, 0) + vp(1, 0);
    p_planes[3].b = vp(3, 1) + vp(1, 1);
    p_planes[3].c = vp(3, 2) + vp(1, 2);
    p_planes[3].d = vp(3, 3) + vp(1, 3);
    // Near clipping plane
    p_planes[4].a = vp(3, 0) + vp(2, 0);
    p_planes[4].b = vp(3, 1) + vp(2, 1);
    p_planes[4].c = vp(3, 2) + vp(2, 2);
    p_planes[4].d = vp(3, 3) + vp(2, 3);
    // Far clipping plane
    p_planes[5].a = vp(3, 0) - vp(2, 0);
    p_planes[5].b = vp(3, 1) - vp(2, 1);
    p_planes[5].c = vp(3, 2) - vp(2, 2);
    p_planes[5].d = vp(3, 3) - vp(2, 3);

    /*
    p_planes[0].a = vp(0, 3) + vp(0, 0);
    p_planes[0].b = vp(1, 3) + vp(1, 0);
    p_planes[0].c = vp(2, 3) + vp(2, 0);
    p_planes[0].d = vp(3, 3) + vp(3, 0);
    // Right clipping plane
    p_planes[1].a = vp(0, 3) - vp(0, 0);
    p_planes[1].b = vp(1, 3) - vp(1, 0);
    p_planes[1].c = vp(2, 3) - vp(2, 0);
    p_planes[1].d = vp(3, 3) - vp(3, 0);
    // Top clipping plane
    p_planes[2].a = vp(0, 3) - vp(0, 1);
    p_planes[2].b = vp(1, 3) - vp(1, 1);
    p_planes[2].c = vp(2, 3) - vp(2, 1);
    p_planes[2].d = vp(3, 3) - vp(3, 1);
    // Bottom clipping plane
    p_planes[3].a = vp(0, 3) + vp(0, 1);
    p_planes[3].b = vp(1, 3) + vp(1, 1);
    p_planes[3].c = vp(2, 3) + vp(2, 1);
    p_planes[3].d = vp(3, 3) + vp(3, 1);
    // Near clipping plane
    p_planes[4].a = vp(0, 3) + vp(0, 2);
    p_planes[4].b = vp(1, 3) + vp(1, 2);
    p_planes[4].c = vp(2, 3) + vp(2, 2);
    p_planes[4].d = vp(3, 3) + vp(3, 2);
    // Far clipping plane
    p_planes[5].a = vp(0, 3) - vp(0, 2);
    p_planes[5].b = vp(1, 3) - vp(1, 2);
    p_planes[5].c = vp(2, 3) - vp(2, 2);
    p_planes[5].d = vp(3, 3) - vp(3, 2);
    */

    QVector<QVector4D> pl;
    for(int i = 0; i < 6; ++i) {
        normalizeFPlane(p_planes[i]);
        pl.append(QVector4D(-p_planes[i].a, -p_planes[i].b, -p_planes[i].c, -p_planes[i].d));
    }

    return pl;
}

bool FrustumUtils::intersects(const QVector<QVector4D> &pl, const QVector3D &pos, float halfSize) {
    for(QVector<QVector4D>::ConstIterator p = pl.begin(); p != pl.end(); ++p) {
        float e = halfSize * (qAbs(p->x()) + qAbs(p->y()) + qAbs(p->z()));
        float s = QVector3D::dotProduct(pos, p->toVector3D()) + p->w();
        if(s - e > 0) return false;
    }
    return true;
}

QVector<int> FrustumUtils::getIntersections(const QMatrix4x4 &vp, const QVector3D &camPos, float cubeSize) {
    QVector<QVector4D> planes = extractPlanes(vp);

    QVector<int> octs;
    int i = 0;
    float halfSize = cubeSize / 2.0;
    float adjSize = halfSize - 0.1;

    if(intersects(planes, camPos + QVector3D(-halfSize, halfSize, -halfSize), adjSize)) octs.append(i);
    ++i;
    if(intersects(planes, camPos + QVector3D(halfSize, halfSize, -halfSize), adjSize)) octs.append(i);
    ++i;
    if(intersects(planes, camPos + QVector3D(-halfSize, halfSize, halfSize), adjSize)) octs.append(i);
    ++i;
    if(intersects(planes, camPos + QVector3D(halfSize, halfSize, halfSize), adjSize)) octs.append(i);
    ++i;

    if(intersects(planes, camPos + QVector3D(-halfSize, -halfSize, -halfSize), adjSize)) octs.append(i);
    ++i;
    if(intersects(planes, camPos + QVector3D(halfSize, -halfSize, -halfSize), adjSize)) octs.append(i);
    ++i;
    if(intersects(planes, camPos + QVector3D(-halfSize, -halfSize, halfSize), adjSize)) octs.append(i);
    ++i;
    if(intersects(planes, camPos + QVector3D(halfSize, -halfSize, halfSize), adjSize)) octs.append(i);
    ++i;

    return octs;
}

int FrustumUtils::getIntersectionsAsInt(const QMatrix4x4 &vp, const QVector3D &camPos, float cubeSize) {
    QVector<QVector4D> planes = extractPlanes(vp);

    int res = 0;
    float halfSize = cubeSize / 2.0;
    float adjSize = halfSize - 0.1;

    res |= intersects(planes, camPos + QVector3D(-halfSize, halfSize, -halfSize), adjSize) << 0;
    res |= intersects(planes, camPos + QVector3D(halfSize, halfSize, -halfSize), adjSize) << 1;
    res |= intersects(planes, camPos + QVector3D(-halfSize, halfSize, halfSize), adjSize) << 2;
    res |= intersects(planes, camPos + QVector3D(halfSize, halfSize, halfSize), adjSize) << 3;
    res |= intersects(planes, camPos + QVector3D(-halfSize, -halfSize, -halfSize), adjSize) << 4;
    res |= intersects(planes, camPos + QVector3D(halfSize, -halfSize, -halfSize), adjSize) << 5;
    res |= intersects(planes, camPos + QVector3D(-halfSize, -halfSize, halfSize), adjSize) << 6;
    res |= intersects(planes, camPos + QVector3D(halfSize, -halfSize, halfSize), adjSize) << 7;

    return res;
}
