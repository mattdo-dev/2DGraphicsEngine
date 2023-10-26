/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */


#include <array>

#include "include/GMath.h"
#include "include/GPoint.h"
#include "include/GRect.h"
#include "clip.h"
#include "edge.h"

Clipper::Clipper(const GIRect& bounds) : fBounds(bounds) {}

u_int8_t Clipper::compute_bounds_code(GPoint point) {
    bool left = point.x < this->fBounds.left;
    bool right = point.x > this->fBounds.right;
    bool top = point.y < this->fBounds.top;
    bool bottom = point.y > this->fBounds.bottom;

    if (left) {
        if (top) return TLC;
        if (bottom) return BLC;
        return LEFT;
    }
    if (right) {
        if (top) return TRC;
        if (bottom) return BRC;
        return RIGHT;
    }
    if (top) return TOP;
    if (bottom) return BOTTOM;

    return IN;
}

void Clipper::batch_clip(const GPoint* points, int count, Edge** out, int& out_count) {
    assert(count >= 3);

    *out = new Edge[count + (count >> 1)];
    out_count = 0;

    // up to almost last
    int r_p0_y = GRoundToInt(points[0].y);
    for (int i = 0; i < count - 1; ++i) {
        GPoint p0 = points[i];
        GPoint p1 = points[i + 1];
        int r_p1_y = GRoundToInt(p1.y);
        if (r_p0_y != r_p1_y)
            this->clip_points(p0, p1, out, out_count);
        r_p0_y = r_p1_y;
    }

    GPoint p0 = points[count - 1];
    GPoint p1 = points[0];
    int r_p1_y = GRoundToInt(p1.y);
    if (r_p0_y != r_p1_y)
        this->clip_points(p0, p1, out, out_count);
}

void Clipper::clip_points(GPoint &p0, GPoint &p1, Edge** out, int &out_count) {
    int wind = 1;

    if (p0.y > p1.y) {
        std::swap(p0, p1);
        wind = -wind;
    }

    u_int8_t p0_flag = this->compute_bounds_code(p0);
    u_int8_t p1_flag = this->compute_bounds_code(p1);

    // If either endpoint is entirely out of bounds vertically, exit early
    if (p1_flag == TOP || p0_flag == BOTTOM) {
        return;
    }

    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;

    // Adjust p0 if it's above the top bound
    if (p0_flag & TOP) {
        float x_new = p0.x + dx * (fBounds.top - p0.y) / dy;
        p0 = {x_new, fBounds.top};
    }

    // Adjust p1 if it's below the bottom bound
    if (p1_flag & BOTTOM) {
        float x_new = p1.x - dx * (p1.y - fBounds.bottom) / dy;
        p1 = {x_new, fBounds.bottom};
    }

    // Sort points based on x-coordinate
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }

    // Handle left and right bounds
    if (p1_flag & LEFT) {
        out[out_count++] = make_edge({fBounds.left, p0.y}, {fBounds.left, p1.y}, wind);
        return;
    }
    if (p0_flag & RIGHT) {
        out[out_count++] = make_edge({fBounds.right, p0.y}, {fBounds.right, p1.y}, wind);
        return;
    }

    // Adjust p0 if it's to the left of the left bound
    if (p0_flag & LEFT) {
        float y_new = p0.y + dy * (fBounds.left - p0.x) / dx;
        out[out_count++] = make_edge({fBounds.left, p0.y}, {fBounds.left, y_new}, wind);
        p0 = {fBounds.left, y_new};
    }

    // Adjust p1 if it's to the right of the right bound
    if (p1_flag & RIGHT) {
        float y_new = p1.y - dy * (p1.x - fBounds.right) / dx;
        out[out_count++] = make_edge({fBounds.right, y_new}, {fBounds.right, p1.y}, wind);
        p1 = {fBounds.right, y_new};
    }

    (*out)[out_count++] = make_edge({fBounds.left, p0.y}, {fBounds.left, p1.y}, wind);

}
