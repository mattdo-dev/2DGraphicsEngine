/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */


#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GPixel.h"
#include "include/GPoint.h"
#include "utils.h"

class BitmapShader : public GShader {
private:
    GBitmap fBitmap;
    GMatrix fLocInv;
    GMatrix fInverse;
public:
    BitmapShader(const GBitmap &bitmap, const GMatrix &localInverse) : fBitmap(bitmap), fLocInv(localInverse) {}

    bool isOpaque() override {
        return this->fBitmap.isOpaque();
    }

    bool setContext(const GMatrix &ctm) override {
        return ctm.invert(&fInverse) && (fInverse = fLocInv * fInverse, true);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        if (fBitmap.width() <= 0 || fBitmap.height() <= 0) return;

        GPoint p = fInverse * GPoint{(float) x + 0.5f, (float) y + 0.5f};
        int p_pr_x, p_pr_y;

        /*
         * On transformations that are not rotations, we can observe an optimization that lifts
         * the need to recompute p.y per iteration.
         */
        bool y_compute = std::abs(fInverse[3]) > std::numeric_limits<float>::epsilon();
        if (!y_compute) {
            p_pr_y = clamp_and_floor(p.y, fBitmap.height());
        }

        for (int i = 0; i < count; ++i) {
            p_pr_x = clamp_and_floor(p.x, fBitmap.width());

            if (y_compute) {
                p_pr_y = clamp_and_floor(p.y, fBitmap.height());
            }

            row[i] = *fBitmap.getAddr(p_pr_x, p_pr_y);

            p.x += fInverse[0];

            if (y_compute) {
                p.y += fInverse[3];
            }
        }
    }

};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap &bitmap,
                                             const GMatrix &localInverse) {
    if (!bitmap.pixels()) return nullptr;
    return std::make_unique<BitmapShader>(bitmap, localInverse);
}
