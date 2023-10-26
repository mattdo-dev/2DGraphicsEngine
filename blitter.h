//
// Created by mattdo on 10/14/23.
//

#ifndef BLITTER_H_
#define BLITTER_H_

#include "include/GBitmap.h"
#include "include/GRect.h"
#include "include/GPaint.h"

class Blit {
private:
    const GBitmap fDevice;
    const GRect fBounds;
    const GPaint fPaint;
    GPixel* fBuffer;  // pre-allocated buffer

public:
    Blit(const GBitmap&, const GRect&, const GPaint&);
    ~Blit();
    void blit_horizontal(float x_left, float x_right, int y);
};

#endif
