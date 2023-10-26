/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */


#ifndef CLIP_H_
#define CLIP_H_

#include <array>

#include "include/GPoint.h"
#include "include/GRect.h"
#include "edge.h"

const u_int8_t IN = 0;
const u_int8_t TOP = 1;
const u_int8_t BOTTOM = 2;
const u_int8_t LEFT = 4;
const u_int8_t RIGHT = 8;
const u_int8_t CORNER = 16;
const u_int8_t TLC = TOP | LEFT | CORNER;
const u_int8_t TRC = TOP | RIGHT | CORNER;
const u_int8_t BLC = BOTTOM | LEFT | CORNER;
const u_int8_t BRC = BOTTOM | RIGHT | CORNER;

class Clipper {
private:
    const GIRect &fBounds;
protected:
    void clip_points(GPoint &, GPoint &, Edge**, int &);
public:
    Clipper(const GIRect &);

    u_int8_t compute_bounds_code(GPoint);

    void batch_clip(const GPoint *, int, Edge**, int &);
};


#endif
