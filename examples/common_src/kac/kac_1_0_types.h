/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Data types of the KAC 1.0 file format.
 * 
 */

#ifndef KAC_1_0_TYPES_H
#define KAC_1_0_TYPES_H

#include <shiet/common/stdint.h>

#define KAC_1_0_VERSION_VALUE 1.0

/* KAC 1.0 requires textures to be at most 256 x 256 and at least 1 x 1, so in
 * that range we can have up to 9 progressively size-halved mip levels (counting
 * the original size).
 *
 * If you modify this value (for another version of KAC), you should also change
 * the constants on texture dimensions.
 */
#define KAC_1_0_MAX_NUM_MIP_LEVELS 9

/* KAC 1.0 requires textures to be square and power-of-two.
 *
 * If you modify these value (for another version of KAC), you should also change
 * the constant on maximum number of mip levels, accordingly.
 */
#define KAC_1_0_MAX_TEXTURE_SIDE_LENGTH 256
#define KAC_1_0_MIN_TEXTURE_SIDE_LENGTH 1

struct kac_1_0_texture_s
{
    struct kac_1_0_texture_metadata_s
    {
        unsigned sideLength : 16;
        unsigned padding : 16;
        uint8_t pixelHash[16]; /* 128 bits of a hash of the texture's pixel data.*/
    } metadata;

    struct kac_1_0_texture_pixel_s
    {
        unsigned r : 5;
        unsigned g : 5;
        unsigned b : 5;
        unsigned a : 1;
    } *mipLevel[KAC_1_0_MAX_NUM_MIP_LEVELS];

    unsigned numMipLevels;
};

struct kac_1_0_material_s
{
    struct kac_1_0_material_color_s
    {
        unsigned r : 4;
        unsigned g : 4;
        unsigned b : 4;
        unsigned a : 4;
    } color;

    struct kac_1_0_material_metadata_s
    {
        unsigned textureIdx : 16;
        unsigned hasTexture : 1;
        unsigned hasTextureFiltering : 1;
        unsigned hasSmoothShading : 1;
        unsigned padding : 13;
    } metadata;
};

struct kac_1_0_vertex_coordinates_s
{
    float x;
    float y;
    float z;
};

struct kac_1_0_vertex_s
{
    uint16_t vertexCoordinatesIdx;
    uint16_t normalIdx;
    uint16_t uvIdx;
};

struct kac_1_0_triangle_s
{
    uint16_t materialIdx;
    struct kac_1_0_vertex_s vertices[3];
};

struct kac_1_0_normal_s
{
    float x;
    float y;
    float z;
};

struct kac_1_0_uv_coordinates_s
{
    float u;
    float v;
};

#endif
