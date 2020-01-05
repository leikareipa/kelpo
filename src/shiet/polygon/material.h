#ifndef SHIET_POLYGON_MATERIAL_H_
#define SHIET_POLYGON_MATERIAL_H_

#include <shiet/polygon/texture.h>
#include <shiet/common/stdint.h>

struct shiet_polygon_material_s
{
    /* If the texture pointer is NULL, baseColor will be used instead.*/
    struct shiet_polygon_texture_s *texture;

    /* E.g. red/green/blue/alpha.*/
    uint8_t baseColor[4];
};

#endif
