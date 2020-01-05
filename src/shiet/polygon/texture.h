#ifndef SHIET_POLYGON_TEXTURE_H_
#define SHIET_POLYGON_TEXTURE_H_

#include <shiet/common/stdint.h>

struct shiet_polygon_texture_s
{
    unsigned width, height;

    /* What kind of filtering to ask the renderer to apply to the texture. The
     * renderer will honor this request if it can; otherwise, it is free to pick
     * a more suitable filtering method.*/
    enum
    {
        SHIET_TEXTURE_FILTER_LINEAR,
        SHIET_TEXTURE_FILTER_NEAREST
    } filtering;

    /* The texture's pixels in 16-bit ARGB 1555 format for each mip level. Level
     * 0 is the base texture; each further mip level is one half of the previous
     * level in its dimensions, so e.g. #0: 128 x 128, #1: 64 x 64, #2: 32 x 32,
     * all the way down to 1 x 1. There can be at most 8 mip levels, so each
     * texture must have a side length of at most 256 pixels. Textures smaller
     * than 256 pixels per side do not need (nor use) all of the 8 mip levels.*/
    uint16_t *mipLevel[8];
    unsigned numMipLevels;
    
    /* A value that identifies this texture with the specific render API used.
     * For instance, with OpenGL, this might be the value generated by a call
     * to glGenTextures().*/
    int32_t apiId;
};

#endif
