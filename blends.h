/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */


#ifndef BLENDS_H_
#define BLENDS_H_

#include "include/GColor.h"
#include "include/GPixel.h"
#include "include/GBlendMode.h"
#include "utils.h"

/* GPixel data structure is an u_int32_t A - R - G - B format, where each
 * component is 8 bits.
 * A << 24
 * R << 16
 * G << 8
 * B << 0
 * While we are unable to take advantage of fastDivide255, we can still use
 * an optimal division algorithm for 255.
 */

typedef u_int64_t ExpandedGPixel;
typedef u_int32_t CompactGPixel; // nomenclature discrete from GPixel

const u_int64_t GPIXEL_EXPANDED_128 = 0x0080008000800080; // 128 per channel
const u_int64_t GPIXEL_EXPANDED_255 = 0x00FF00FF00FF00FF; // 255 per channel
const u_int64_t A_G_ = 0xFF00FF00;
const u_int64_t _R_B = 0x00FF00FF;

/*
 * Suppose x = 0xAARRGGBB.
 * After typing conversion, we have:
 * a = x & 0xFF00FF00 ==> 0x00000000AARRGGBB
 * a = x << 24        ==> 0x00AA00GG00000000
 * b = x & 0x00FF00FF ==> 0x0000000000RR00BB
 * a | b              ==> 0x00AA00GG00RR00BB
 * The format is out of order, but since the math doesn't matter on order as
 * long as it is reversible and the "colors" are distinct from each other
 * to avoid interference in calculations.
 *
 * The division step is simply the +128->/255 optimization step, but with
 * the advantage of being able to calculate all channels at once.
 *  */

// automatic type conversion
static inline ExpandedGPixel expandPixel(u_int64_t x) {
    return ((x & A_G_) << 24) | (x & _R_B);
}

static inline CompactGPixel compactGPixel(ExpandedGPixel x) {
    return ((x >> 24) & A_G_) | (x & _R_B);
}

static inline CompactGPixel fullPixelMulDivide255(CompactGPixel x, u_int8_t prod) {
    ExpandedGPixel res = expandPixel(x) * prod + GPIXEL_EXPANDED_128;
    res += (res >> 8) & GPIXEL_EXPANDED_255;
    return compactGPixel(res >> 8);
}

static inline GPixel combineAlphaAndRGB(const GPixel &a, const GPixel &rgb) {
    return (a << GPIXEL_SHIFT_A) | (rgb & 0x00FFFFFF);
}

/* This division method only works for discrete values of 255; for handling
 * pixels in its entirety, see above
 */
static inline uint8_t fastDivide255(unsigned prod) {
    const unsigned coefficient = (1 << 16) | (1 << 8) | 1;
    return (prod * coefficient + (1 << 23)) >> 24;
}

static inline GPixel clearBlend(const GPixel &dst, const GPixel &src) {
    (void) src;
    (void) dst;
    return 0;
}

static inline GPixel srcBlend(const GPixel &dst, const GPixel &src) {
    (void) dst;
    return src;
}

static inline GPixel dstBlend(const GPixel &dst, const GPixel &src) {
    (void) src;
    return dst;
}

static inline GPixel srcOverBlend(const GPixel &dst, const GPixel &src) {
    unsigned src_a = GPixel_GetA(src);
    // ~src_a = 255 - src_a
    return src + fullPixelMulDivide255(dst, ~src_a);
}

static inline GPixel dstOverBlend(const GPixel &dst, const GPixel &src) {
    // Trivial swap, shared functionality
    unsigned dst_a = GPixel_GetA(dst);

    return dst + fullPixelMulDivide255(src, ~dst_a);
}

static inline GPixel srcInBlend(const GPixel &dst, const GPixel &src) {
    unsigned dst_a = GPixel_GetA(dst);

    return fullPixelMulDivide255(src, dst_a);
}

static inline GPixel dstInBlend(const GPixel &dst, const GPixel &src) {
    unsigned src_a = GPixel_GetA(src);

    return fullPixelMulDivide255(dst, src_a);
}

static inline GPixel srcOutBlend(const GPixel &dst, const GPixel &src) {
    unsigned dst_a = GPixel_GetA(dst);

    return fullPixelMulDivide255(src, ~dst_a);
}

static inline GPixel dstOutBlend(const GPixel &dst, const GPixel &src) {
    unsigned src_a = GPixel_GetA(src);

    return fullPixelMulDivide255(dst, ~src_a);
}

static inline GPixel srcATopBlend(const GPixel &dst, const GPixel &src) {
    unsigned src_a = GPixel_GetA(src);
    unsigned dst_a = GPixel_GetA(dst);

    CompactGPixel a = fullPixelMulDivide255(src, dst_a);
    CompactGPixel b = fullPixelMulDivide255(dst, ~src_a);

    return combineAlphaAndRGB(dst_a, a + b);
}

static inline GPixel dstATopBlend(const GPixel &dst, const GPixel &src) {
    unsigned src_a = GPixel_GetA(src);
    unsigned dst_a = GPixel_GetA(dst);

    CompactGPixel a = fullPixelMulDivide255(dst, src_a);
    CompactGPixel b = fullPixelMulDivide255(src, ~dst_a);

    return combineAlphaAndRGB(src_a, a + b);
}

static inline GPixel xorBlend(const GPixel &dst, const GPixel &src) {
    uint8_t src_a = GPixel_GetA(src);
    uint8_t dst_a = GPixel_GetA(dst);

    if (dst_a == 0) return src;
    if (dst_a == 255) return dstOutBlend(dst, src);

    CompactGPixel a = fullPixelMulDivide255(dst, ~src_a);
    CompactGPixel b = fullPixelMulDivide255(src, ~dst_a);
    return a + b;
}

typedef GPixel (*BlendFuncPtr)(const GPixel &dst, const GPixel &src);

enum Alpha {
    ZERO,
    TRANSLUCENT,
    OPAQUE,
};

// Number of blending modes and alpha categories
#define NUM_MODES 12
#define NUM_ALPHA_CATEGORIES 3

const BlendFuncPtr blend_lut[NUM_MODES][NUM_ALPHA_CATEGORIES] = {
        // kClear
        {clearBlend, clearBlend, clearBlend},
        // kSrc
        {clearBlend, srcBlend, srcBlend},
        // kDst
        {dstBlend, dstBlend, dstBlend},
        // kSrcOver
        {dstBlend, srcOverBlend, srcBlend},
        // kDstOver
        {dstBlend, dstOverBlend, dstOverBlend},
        // kSrcIn
        {clearBlend, srcInBlend, srcInBlend},
        // kDstIn
        {clearBlend, dstInBlend, dstInBlend},
        // kSrcOut
        {clearBlend, srcOutBlend, srcOutBlend},
        // kDstOut
        {dstBlend, dstOutBlend, clearBlend},
        // kSrcATop
        {dstBlend, srcATopBlend, srcATopBlend},
        // kDstATop
        {clearBlend, dstATopBlend, dstATopBlend},
        // kXor
        {dstBlend, xorBlend, srcOutBlend}
};

static inline BlendFuncPtr blend_func(const GBlendMode mode, const GPixel &src) {
    // Determine the category of alpha
    uint8_t src_a = GPixel_GetA(src);
    Alpha alpha_category;

    if (src_a == 0) {
        alpha_category = ZERO;
    } else if (src_a == 255) {
        alpha_category = OPAQUE;
    } else {
        alpha_category = TRANSLUCENT;
    }

    // Retrieve the blend function from the lookup table
    return blend_lut[static_cast<int>(mode)][static_cast<int>(alpha_category)];
}


#endif // BLENDS_H_