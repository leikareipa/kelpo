#ifndef SHIET_POLYGON_TRIANGLE_H_
#define SHIET_POLYGON_TRIANGLE_H_

#include "shiet/polygon/material.h"
#include "shiet/polygon/vertex.h"

struct shiet_polygon_triangle_s
{
    struct shiet_polygon_vertex_s vertex[3];
    struct shiet_polygon_material_s material;
};

#endif
