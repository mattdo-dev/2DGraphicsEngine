/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */


#include <cmath>

#include "include/GMatrix.h"
#include "include/GPoint.h"

GMatrix::GMatrix() : fMat{1, 0, 0,
                          0, 1, 0} {}

GMatrix GMatrix::Translate(float tx, float ty) {
    return {1.0, 0.0, tx,
            0.0, 1.0, ty};
}

GMatrix GMatrix::Scale(float sx, float sy) {
    return {sx, 0.0, 0.0,
            0.0, sy, 0.0};
}

GMatrix GMatrix::Rotate(float radians) {
    return {cosf(radians), -sinf(radians), 0.0,
            sinf(radians), cosf(radians), 0.0};
}

GMatrix GMatrix::Concat(const GMatrix &a, const GMatrix &b) {
    float ma = a.fMat[0] * b.fMat[0] + a.fMat[1] * b.fMat[3];
    float mb = a.fMat[0] * b.fMat[1] + a.fMat[1] * b.fMat[4];
    float mc = a.fMat[0] * b.fMat[2] + a.fMat[1] * b.fMat[5] + a.fMat[2];
    float md = a.fMat[3] * b.fMat[0] + a.fMat[4] * b.fMat[3];
    float me = a.fMat[3] * b.fMat[1] + a.fMat[4] * b.fMat[4];
    float mf = a.fMat[3] * b.fMat[2] + a.fMat[4] * b.fMat[5] + a.fMat[5];
    return {ma, mb, mc,
            md, me, mf};
}

bool GMatrix::invert(GMatrix *inverse) const {
    float det = fMat[0] * fMat[4] - fMat[1] * fMat[3];
    if (det == 0) return false;

    // precompute
    float invDet = 1 / det;
    float a = fMat[4] * invDet;
    float b = -fMat[1] * invDet;
    float c = (fMat[1] * fMat[5] - fMat[4] * fMat[2]) * invDet;
    float d = -fMat[3] * invDet;
    float e = fMat[0] * invDet;
    float f = (fMat[3] * fMat[2] - fMat[0] * fMat[5]) * invDet;

    inverse->fMat[0] = a;
    inverse->fMat[1] = b;
    inverse->fMat[2] = c;
    inverse->fMat[3] = d;
    inverse->fMat[4] = e;
    inverse->fMat[5] = f;
    return true;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; ++i) {
        dst[i] = {fMat[0] * src[i].x + fMat[1] * src[i].y + fMat[2],
                  fMat[3] * src[i].x + fMat[4] * src[i].y + fMat[5]};
    }
}
