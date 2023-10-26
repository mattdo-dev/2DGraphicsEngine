/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */

#include "include/GPath.h"
#include "include/GMatrix.h"

template <unsigned N> class PointsIterator {
private:
    unsigned idx;
    unsigned dir;
protected:
    GPoint points[N];
public:
    PointsIterator(GPath::Direction direction)
    : idx(0), dir(direction == GPath::kCW_Direction ? 1 : N - 1) {}

    const GPoint& current() {
        return points[idx];
    }

    const GPoint& next() {
        idx = (idx + dir) % N;
        return this->current();
    }
};

class GPath_RectIterator : public PointsIterator<4> {
public:
    GPath_RectIterator(const GRect& rect, GPath::Direction direction) : PointsIterator(direction) {
        points[0] = {rect.left, rect.top};
        points[1] = {rect.right, rect.top};
        points[2] = {rect.right, rect.bottom};
        points[3] = {rect.left, rect.bottom};
    }
};

void GPath::addPolygon(const GPoint *pts, int count) {
    if (count < 2) return;

    this->moveTo(pts[0]);
    for (int i = 1; i < count; ++i) {
        this->lineTo(pts[i]);
    }
}

void GPath::addRect(const GRect& rect, GPath::Direction direction) {
    GPath_RectIterator it(rect, direction);

    this->moveTo(it.current());
    this->lineTo(it.next());
    this->lineTo(it.next());
    this->lineTo(it.next());
}

GRect GPath::bounds() const {
    int count = this->fPts.size();

    if (count < 1) return GRect::WH(0, 0);

    float x_min = this->fPts[0].x;
    float y_min = this->fPts[0].y;
    float x_max = x_min;
    float y_max = y_min;

    for (int i = 1; i < count; i++) {
        const GPoint& p = this->fPts[i];

        x_min = std::min(x_min, p.x);
        x_max = std::max(x_max, p.x);
        y_min = std::min(y_min, p.y);
        y_max = std::max(y_max, p.y);
    }

    return GRect::LTRB(x_min, y_min, x_max, y_max);
}


void GPath::transform(const GMatrix& matrix) {
    matrix.mapPoints(this->fPts.data(), this->fPts.data(), this->fPts.size());
}