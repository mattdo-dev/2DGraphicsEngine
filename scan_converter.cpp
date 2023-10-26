/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */

#include "include/GPath.h"
#include "include/GMath.h"
#include "scan_converter.h"
#include "edge.h"

void ScanConverter::scan_rect(GIRect& rect, Blit& blit) {
    for (int y = std::max(0, rect.top); y < rect.bottom; ++y) {
        blit.blit_horizontal(rect.left, rect.right, y);
    }
}

void ScanConverter::scan_convex(Edge *edges, int count, Blit& blit) {
    if (count < 2) return;

    std::sort(edges, edges + count, compare_edge);

    int y_last = edges[count - 1].y_min;

    Edge *left = &edges[0];
    Edge *right = &edges[1];
    int edge_idx = 2;

    int y_cur = left->y_max;
    float x_left = left->cur_x;
    float x_right = right->cur_x;

    // iterate, and blit
    while (y_cur < y_last) {
        blit.blit_horizontal(x_left, x_right, y_cur++);

        if (left->y_min <= y_cur) {
            left = &edges[edge_idx++];
            x_left = left->cur_x;
        } else {
            x_left += left->m;
        }

        if (right->y_min <= y_cur) {
            right = &edges[edge_idx++];
            x_right = right->cur_x;
        } else {
            x_right += right->m;
        }
    }
}

//void ScanConverter::scan_complex(Edge *edges, int count, Blit &blit) {
//    if (count < 2) return;
//
//    std::sort(edges, edges + count, compare_edge);
//
//    int y_top = edges->y_max;
//    int y_bot = (edges + count - 1)->y_min;
//
//    for (int y = y_top; y <= y_bot; y++) {
//        std::vector<int> intersections;
//
//        for (int i = 0; i < count; ) {
//            Edge& edge = *(edges + i);
//
//            if (y >= edge.y_max && y < edge.y_min) {
//                intersections.push_back(GRoundToInt(edge.cur_x));
//                edge.cur_x += edge.m;
//            }
//
//            if (y >= edge.y_min) {
//                // Shift the edges to fill the gap
//                std::swap(*(edges + i), *(edges + count - 1));
//                count--;
//            } else {
//                i++;
//            }
//        }
//
//        std::sort(intersections.begin(), intersections.end());
//
//        for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
//            blit.blit_horizontal(intersections[i], intersections[i + 1], y);
//        }
//    }
//}

void ScanConverter::scan_complex(Edge *edges, int count, Blit &blit) {
    if (count < 2) return;

    std::sort(edges, edges + count, compare_edge);

    int y = edges->y_max;

    while (count > 0) {
        int curr = 0, x0 = 0, x1, winding = 0;

        while (curr < count && edges[curr].y_max <= y) {
            if (winding == 0)
                x0 = GRoundToInt(edges[curr].cur_x);

            winding += edges[curr].wind;

            if (winding == 0) {
                x1 = GRoundToInt(edges[curr].cur_x);
                blit.blit_horizontal(x0, x1, y);
            }

            if (y + 1 >= edges[curr].y_min) {
                memcpy(&edges[curr], &edges[curr + 1], sizeof(Edge) * (count - curr));
                count--;
            } else {
                edges[curr].cur_x += edges[curr].m;
                curr++;
            }
        }

        y++;

        while (curr < count && y == edges[curr].y_max) curr++;

        std::sort(edges, edges + curr, [](const Edge& a, const Edge& b) -> bool {
            return a.cur_x < b.cur_x;
        });
    }
}
