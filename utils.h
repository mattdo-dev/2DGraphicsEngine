/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "include/GColor.h"
#include "include/GPaint.h"
#include "include/GPixel.h"
#include "include/GMatrix.h"

static inline GPixel color_to_pixel(const GColor &color) {
    if (color.a == 0) return 0;
    unsigned a = GRoundToInt(color.a * 255);
    unsigned r = GRoundToInt(color.r * color.a * 255);
    unsigned g = GRoundToInt(color.g * color.a * 255);
    unsigned b = GRoundToInt(color.b * color.a * 255);
    return GPixel_PackARGB(a, r, g, b);
}

static inline bool null_draw(const GPaint &paint) {
    if (paint.getShader()) return false;
    switch (paint.getBlendMode()) {
        case GBlendMode::kSrcOver:
        case GBlendMode::kDstOut:
        case GBlendMode::kDstOver:
            return paint.getAlpha() == 0;
        case GBlendMode::kDst:
            return true;
        default:
            break;
    }
    return false;
}

static inline float clamp(float x, float max) {
    if (x > max) return max;
    if (x < 0.0f) return 0.0f;
    return x;
}

static inline int clamp_and_floor(float value, int max) {
    return std::max(0, std::min(GFloorToInt(value), max - 1));
}

static inline bool unrotated(GMatrix& matrix) {
    return (matrix[1] == 0.0f || matrix[3] == 0.0f);
}

static inline GIRect rect_from_points(const GPoint pts[4]) {
    float min_x = std::min({pts[0].x, pts[1].x, pts[2].x, pts[3].x});
    float max_x = std::max({pts[0].x, pts[1].x, pts[2].x, pts[3].x});
    float min_y = std::min({pts[0].y, pts[1].y, pts[2].y, pts[3].y});
    float max_y = std::max({pts[0].y, pts[1].y, pts[2].y, pts[3].y});

    return GIRect::LTRB(GRoundToInt(min_x), GRoundToInt(min_y),
                        GRoundToInt(max_x), GRoundToInt(max_y));
}

static inline GIRect clip_to_bounds(const GIRect& rect, const GRect& bounds) {
    return GIRect::LTRB(
            std::max(rect.left, GRoundToInt(bounds.left)),
            std::max(rect.top, GRoundToInt(bounds.top)),
            std::min(rect.right, GRoundToInt(bounds.right)),
            std::min(rect.bottom, GRoundToInt(bounds.bottom))
    );
}

#endif
