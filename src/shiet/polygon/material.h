#ifndef SHIET_POLYGON_MATERIAL_H_
#define SHIET_POLYGON_MATERIAL_H_

#include "shiet/polygon/texture.h"
#include "shiet/common/types.h"

struct shiet_polygon_material_s
{
    /* If the texture pointer is NULL, baseColor will be used instead.*/
    const shiet_polygon_texture_s *texturePtr;

    shiet_palette_idx_t baseColor;
};

#endif
