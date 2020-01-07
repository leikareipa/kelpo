#ifndef SHIET_POLYGON_VERTEX_H
#define SHIET_POLYGON_VERTEX_H

#include <shiet_interface/common/stdint.h>

/* Note: Changes to this struct should also be reflected in the Glide 3.x renderer's
 * calls to grVertexLayout().*/
struct shiet_polygon_vertex_s
{
    /* World coordinates.*/
    float x, y, z, w;

    /* Normal.*/
    float nx, ny, nz;

    /* Texture coordinates.*/
    float u, v;

    /* Color.*/
    uint8_t r, g, b, a;
};

#endif
