#ifndef SHIET_POLYGON_VERTEX_H_
#define SHIET_POLYGON_VERTEX_H_

#include "shiet/common/types.h"

struct shiet_polygon_vertex_s
{
    shiet_fixedpoint_t x, y, z, w;
    real u, v; /// TODO: Use fixed-point.
};

#endif
