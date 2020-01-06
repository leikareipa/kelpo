#ifndef SHIET_POLYGON_VERTEX_H_
#define SHIET_POLYGON_VERTEX_H_

#include <shiet/common/stdint.h>

struct shiet_polygon_vertex_s
{
    /* World coordinates.*/
    float x, y, z, w;

    /* Normal.*/
    float nx, ny, nz;

    /* Texture coordinates.*/
    float u, v;

    /* Color.*/
    uint8_t r, g, b;
};

#endif
