/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */

#include "include/GPaint.h"
#include "include/GPixel.h"
#include "include/GShader.h"
#include "blitter.h"
#include "blends.h"
#include "utils.h"

Blit::Blit(const GBitmap& bitmap, const GRect& bounds, const GPaint& paint)
        : fDevice(bitmap), fBounds(bounds), fPaint(paint) {
    fBuffer = new GPixel[fDevice.width()];  // Allocate buffer based on the bitmap width
    (void) fBounds;
}

Blit::~Blit() {
    delete[] fBuffer;  // Deallocate the buffer
}

void Blit::blit_horizontal(float x_left, float x_right, int y) {
    if (y < 0 || y >= this->fDevice.height()) return;

    int x_l_int = std::max(GRoundToInt(x_left), 0);
    int x_r_int = std::min(GRoundToInt(x_right), this->fDevice.width());

    if (x_l_int >= x_r_int) return;

    GShader *shader = this->fPaint.getShader();

    if (!shader) {
        GPixel src = color_to_pixel(this->fPaint.getColor());
        BlendFuncPtr blend = blend_func(this->fPaint.getBlendMode(),    src);

        for (int i = x_l_int; i < x_r_int; ++i) {
            GPixel *dst = this->fDevice.getAddr(i, y);
            *dst = blend(*dst, src);
        }
    } else {
        int count = x_r_int - x_l_int;
        shader->shadeRow(x_l_int, y, count, fBuffer);

        BlendFuncPtr blend;
        GPixel *dst = this->fDevice.getAddr(x_l_int, y);
        GPixel *src_pixel = fBuffer;

        for (int i = x_l_int; i < x_r_int; ++i, ++dst, ++src_pixel) {
            blend = blend_func(this->fPaint.getBlendMode(), *src_pixel);
            *dst = blend(*dst, *src_pixel);
        }
    }
}
