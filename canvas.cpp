/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */

#include <stack>
#include <iostream>

#include "include/GBitmap.h"
#include "include/GCanvas.h"
#include "include/GColor.h"
#include "include/GPoint.h"
#include "include/GRect.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include "include/GShader.h"
#include "include/GPath.h"
#include "blitter.h"
#include "edge.h"
#include "blends.h"
#include "clip.h"
#include "scan_converter.h"

class OtherCanvas : public GCanvas {
private:
    const GBitmap fDevice;
    GRect fBounds{};
    std::stack<GMatrix> CTMStack;
public:
    explicit OtherCanvas(const GBitmap &device) : fDevice(device) {
        fBounds = GRect::LTRB(0,
                              0,
                              static_cast<int>(device.width()),
                              static_cast<int>(device.height()));
        CTMStack.emplace();
    }

    void clear(const GColor &color) override {
        GPixel src = color_to_pixel(color);

        int height = fDevice.height();
        int width = fDevice.width();

        for (int y = 0; y < height; ++y) {
            GPixel* row_start = fDevice.getAddr(0, y);
            GPixel* row_end = row_start + width;

            for (GPixel* addr = row_start; addr < row_end; ++addr) {
                *addr = src;
            }
        }
    }

    void drawConvexPolygon(const GPoint src_points[], int count, const GPaint &paint) override {
        if (count < 3) return;
        if (null_draw(paint)) return;
        if (paint.getShader() && !paint.getShader()->setContext(this->CTMStack.top())) return;

//        // Check if the points form an unrotated quad
//        if (count == 4 && (this->CTMStack.size() == 0 || unrotated(this->CTMStack.top()))) {
//            float y1_diff = points[0].y - points[1].y;
//            float y2_diff = points[2].y - points[3].y;
//            float x1_diff = points[0].x - points[3].x;
//            float x2_diff = points[1].x - points[2].x;
//
//            if (std::abs(y1_diff) < 1e-5 && std::abs(y2_diff) < 1e-5 &&
//                std::abs(x1_diff) < 1e-5 && std::abs(x2_diff) < 1e-5) {
//
//                // Create a GRect using the min/max x and y values
//                int x_min = std::min({points[0].x, points[1].x, points[2].x, points[3].x});
//                int y_min = std::min({points[0].y, points[1].y, points[2].y, points[3].y});
//                int x_max = std::max({points[0].x, points[1].x, points[2].x, points[3].x});
//                int y_max = std::max({points[0].y, points[1].y, points[2].y, points[3].y});
//
//                this->drawRect(GRect::LTRB(x_min, y_min, x_max, y_max), paint);
//            }
//        }

        GPoint points[count];
        this->CTMStack.top().mapPoints(points, src_points, count);

        Edge *edges;
        int edge_count;

        Clipper clipper = Clipper(this->fBounds);
        clipper.batch_clip(points, count, &edges, edge_count);

        if (edge_count < 2) return;

        Blit blit = Blit(this->fDevice, this->fBounds, paint);

        ScanConverter::scan_convex(edges, edge_count, blit);

        delete[] edges;
    }

    void drawPath(const GPath& path, const GPaint& paint) override {
        if (path.countPoints() < 3) return;
        if (null_draw(paint)) return;
        if (paint.getShader() && !paint.getShader()->setContext(this->CTMStack.top())) return;

        GPath transformed = path;
        transformed.transform(this->CTMStack.top());
        int path_count = transformed.countPoints();

        GPath::Edger edger = GPath::Edger(transformed);
        GPath::Verb verb;

        GPoint pts_list[path_count << 1];
        int tw_idx = 0;

        do {
            verb = edger.next(&pts_list[tw_idx << 1]);
            if (verb == GPath::Verb::kLine)
                tw_idx++;
        } while (verb != GPath::Verb::kDone);

        Edge *edges;
        int edge_count;

        Clipper clipper = Clipper(this->fBounds);
        clipper.batch_clip(pts_list, tw_idx, &edges, edge_count);

        Blit blit = Blit(this->fDevice, this->fBounds, paint);
        ScanConverter::scan_complex(edges, edge_count, blit);

        delete[] edges;
    }


    /**
     * Draw a rectangular area by filling it with the provided paint.
     */
    void drawRect(const GRect &rect, const GPaint &paint) override {
        if (null_draw(paint)) return;
        if (paint.getShader() && !paint.getShader()->setContext(this->CTMStack.top())) return;

        if (this->CTMStack.size() == 0 || unrotated(this->CTMStack.top())) {
            GPoint pts[4] = {
                    {rect.left,  rect.top},
                    {rect.right, rect.top},
                    {rect.right, rect.bottom},
                    {rect.left,  rect.bottom}
            };

            GPoint new_pts[4];
            this->CTMStack.top().mapPoints(new_pts, pts, 4);

            GIRect r_rect = rect_from_points(new_pts);
            r_rect = clip_to_bounds(r_rect, fBounds);

            Blit blit(this->fDevice, this->fBounds, paint);
            ScanConverter::scan_rect(r_rect, blit);
            return;
        }

        // If not rotated, directly draw using the convex polygon method without transformations.
        GPoint pts[4] = {
                {rect.left,  rect.top},
                {rect.right, rect.top},
                {rect.right, rect.bottom},
                {rect.left,  rect.bottom}
        };

        drawConvexPolygon(pts, 4, paint);
    }


    void save() override {
        GMatrix copy(this->CTMStack.top());
        this->CTMStack.push(copy);
    }

    void restore() override {
        assert(!CTMStack.empty());
        CTMStack.pop();
    }

    void concat(const GMatrix &matrix) override {
        this->CTMStack.top() = this->CTMStack.top() * matrix;
    }
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap &device) {
    return std::make_unique<OtherCanvas>(device);
}

std::string GDrawSomething(GCanvas *canvas, GISize size) {
    return "void";
}