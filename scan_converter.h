/*
 *  Copyright 2023 <mattdo@email.unc.edu>
 */

#ifndef SCAN_CONVERTER_H
#define SCAN_CONVERTER_H

#include "include/GPaint.h"
#include "include/GPath.h"
#include "include/GRect.h"
#include "edge.h"
#include "blitter.h"


class ScanConverter {
public:
    static void scan_rect(GIRect&, Blit&);
    static void scan_convex(Edge*, int, Blit&);
    static void scan_complex(Edge*, int, Blit&);
};


#endif
