#ifndef SHIET_POLYGON_MATERIAL_H_
#define SHIET_POLYGON_MATERIAL_H_

#include <stdint.h>
#include "shiet/polygon/texture.h"

struct shiet_polygon_material_s
{
    /* If the texture pointer is NULL, baseColor will be used instead.*/
    const struct shiet_polygon_texture_s *texture;

    /* E.g. red/green/blue/alpha.*/
    uint8_t baseColor[4];
};

#endif
