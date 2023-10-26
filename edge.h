/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */


#ifndef EDGE_H
#define EDGE_H

#include "include/GPoint.h"
#include "include/GMath.h"
#include "include/GPoint.h"

struct Edge {
    int32_t y_max{};
    int32_t y_min{};
    float m{};
    float cur_x{};
    int wind;
};

Edge make_edge(GPoint p0, GPoint p1, int wind);

bool compare_edge(Edge e0, Edge e1);

#endif
