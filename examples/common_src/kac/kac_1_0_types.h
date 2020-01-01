/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Data types of the KAC 1.0 file format.
 * 
 */

#ifndef KAC_1_0_TYPES_H
#define KAC_1_0_TYPES_H

#include <stdint.h>

struct kac_1_0_texture_s
{
    struct kac_1_0_texture_metadata_s
    {
        unsigned sideLengthExponent : 3;
        unsigned pixelDataOffset : 25;
        unsigned unused : 4;
        uint8_t pixelHash[16]; /* 128 bits of a hash of the texture's pixel data.*/
    } metadata;

    struct kac_1_0_texture_pixel_s
    {
        unsigned r : 5;
        unsigned g : 5;
        unsigned b : 5;
        unsigned a : 1;
    } *pixels;
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
        unsigned textureMetadataIdx : 9;
        unsigned hasTexture : 1;
        unsigned hasTextureFiltering : 1;
        unsigned hasSmoothShading : 1;
        unsigned unused : 4;
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
