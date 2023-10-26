/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GPixel.h"
#include "include/GPoint.h"
#include "utils.h"

class LinearGradient : public GShader {
private:
    GColor* fColors;
    GMatrix fInverse;
    GMatrix fUnit;
    int fColorsCount;
    bool fOpaque = true;
public:
    LinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count) {
        this->fColorsCount = count;
        this->fColors = (GColor*) malloc(count * sizeof(GColor));
        memcpy(fColors, colors, count * sizeof (GColor));

        float dx = std::abs(p1.x - p0.x);
        float dy = std::abs(p1.y - p0.y);
        this->fUnit = {dx, -dy, p0.x,
                       dy, dx, p0.y};

        for (int i = 0; i < count; ++i) {
            fColors[i].pinToUnit();
            if (this->fOpaque && fColors[i].a < 1.0f) {
                this->fOpaque = false;
            }
        }

    }

    ~LinearGradient() {
        free(this->fColors);
    }

    bool isOpaque() override {
        return this->fOpaque;
    }

    bool setContext(const GMatrix &ctm) override {
        return (ctm * this->fUnit).invert(&this->fInverse);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint local = fInverse * GPoint{x + .5f, y + .5f};
        float dx = fInverse[0];

        /* Early stopping cases */
        if (fColorsCount == 1) {
            GPixel pixel = color_to_pixel(fColors[0]);
            std::fill(row, row + count, pixel);
        } else if (fColorsCount == 2) {
            GColor c1 = fColors[0];
            GColor c2 = fColors[1];
            for (int i = 0; i < count; ++i) {
                float t = GPinToUnit(local.x);
                float inverse = 1.0f - t;

                row[i] = color_to_pixel(
                        GColor::RGBA(
                                c1.r * inverse + c2.r * t,
                                c1.g * inverse + c2.g * t,
                                c1.b * inverse + c2.b * t,
                                c1.a * inverse + c2.a * t)
                );

                local.x += dx;
            }
        } else {
            /* Generic case */
            float stride = 1.0f / (fColorsCount - 1);

            for (int i = 0; i < count; ++i) {
                float t = GPinToUnit(local.x);

                if (t == 0)
                    row[i] = color_to_pixel(fColors[0]);
                else if (t == 1.0f)
                    row[i] = color_to_pixel(fColors[fColorsCount - 1]);
                else {
                    int idx = static_cast<int>(std::floor(t * (fColorsCount - 1)));
                    float start = idx * stride;

                    GColor c1 = fColors[idx];
                    GColor c2 = fColors[idx + 1];

                    t = GPinToUnit((t - start) / stride);
                    float inverse = 1.0f - t;

                    row[i] = color_to_pixel(
                            GColor::RGBA(
                                    c1.r * inverse + c2.r * t,
                                    c1.g * inverse + c2.g * t,
                                    c1.b * inverse + c2.b * t,
                                    c1.a * inverse + c2.a * t)
                    );
                }

                local.x += dx;
            }
        }
    }
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count) {
    if (count < 1) return nullptr;
    return std::make_unique<LinearGradient>(p0, p1, colors, count);
}