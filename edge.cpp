/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */

#include "edge.h"

Edge make_edge(GPoint p0, GPoint p1, int wind) {
    if (p0.y > p1.y) {
        std::swap(p0, p1);
    }

    int upper_y = GRoundToInt(p0.y);
    int lower_y = GRoundToInt(p1.y);
    float slope = (p1.x - p0.x) / (p1.y - p0.y);

    Edge edge;
    edge.y_max = upper_y;
    edge.y_min = lower_y;
    edge.m = slope;
    edge.cur_x = p0.x + slope * ((float) upper_y - p0.y + 0.5f);
    edge.wind = wind;

    return edge;
}

bool compare_edge(Edge e0, Edge e1) {
    if (e0.y_max < e1.y_max) return true;
    if (e0.y_max == e1.y_max) {
        if (e0.cur_x < e1.cur_x) return true;
        if (e0.cur_x == e1.cur_x) {
            if (e0.m < e1.m) return true;
        }
    }
    return false;
}
