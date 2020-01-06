#ifndef SHIET_POLYGON_TRIANGLE_H
#define SHIET_POLYGON_TRIANGLE_H

#include <shiet/polygon/texture.h>
#include <shiet/polygon/vertex.h>

struct shiet_polygon_triangle_s
{
    struct shiet_polygon_vertex_s vertex[3];
    
    struct shiet_polygon_texture_s *texture;
};

#endif
