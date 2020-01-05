/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Loads a KAC 1.0 mesh into a shiet-compatible format.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <shiet/polygon/triangle/triangle_stack.h>
#include <shiet/polygon/triangle/triangle.h>
#include "kac/import_kac_1_0.h"
#include "load_kac_1_0_mesh.h"

int shiet_load_kac10_mesh(const char *const kacFilename,
                          struct shiet_polygon_triangle_stack_s *dstTriangles,
                          struct shiet_polygon_texture_s **dstTextures,
                          uint32_t *numTextures)
{
    struct kac_1_0_vertex_coordinates_s *kacVertexCoords = NULL;
    struct kac_1_0_uv_coordinates_s *kacUVCoords = NULL;
    struct kac_1_0_material_s *kacMaterials = NULL;
    struct kac_1_0_triangle_s *kacTriangles = NULL;
    struct kac_1_0_texture_s *kacTextures = NULL;
    struct kac_1_0_normal_s *kacNormals = NULL;

    uint32_t numTriangles = 0;
    *numTextures = 0;

    #define RELEASE_TEMPORARY_KAC_BUFFERS {uint32_t i = 0;\
                                           for (i = 0; i < *numTextures; i++)\
                                           {\
                                               free(kacTextures[i].pixels);\
                                           }\
                                           free(kacVertexCoords);\
                                           free(kacUVCoords);\
                                           free(kacMaterials);\
                                           free(kacTriangles);\
                                           free(kacTextures);\
                                           free(kacNormals);}

    if (kac10_reader__open_file(kacFilename) &&
        (numTriangles = kac10_reader__read_triangles(&kacTriangles)) &&
        (*numTextures = kac10_reader__read_textures(&kacTextures)) &&
        kac10_reader__read_vertex_coordinates(&kacVertexCoords) &&
        kac10_reader__read_uv_coordinates(&kacUVCoords) &&
        kac10_reader__read_materials(&kacMaterials) &&
        kac10_reader__read_normals(&kacNormals) &&
        kac10_reader__close_file())
    {
        uint32_t i = 0;

        shiet_tristack_grow(dstTriangles, numTriangles);
        *dstTextures = malloc(*numTextures * sizeof(struct shiet_polygon_texture_s));

        /* Convert the KAC textures into shiet's format.*/
        for (i = 0; i < *numTextures; i++)
        {
            uint32_t p = 0;
            const struct kac_1_0_texture_s *kacTexture = &kacTextures[i];
            const uint32_t textureSideLen = pow(2, (kacTexture->metadata.sideLengthExponent + 1));
            const uint32_t numPixelsInTexture = (textureSideLen * textureSideLen);

            (*dstTextures)[i].width = textureSideLen;
            (*dstTextures)[i].height = textureSideLen;
            (*dstTextures)[i].filtering = SHIET_TEXTURE_FILTER_LINEAR;
            (*dstTextures)[i].pixelArray = malloc(numPixelsInTexture * 4);
            (*dstTextures)[i].pixelArray16bit = malloc(numPixelsInTexture * sizeof((*dstTextures)[i].pixelArray16bit[0]));

            for (p = 0; p < numPixelsInTexture; p++)
            {
                /* Convert from the KAC RGBA 5551 format into full-color 8888.*/
                {
                    /* 4 color channels per pixel.*/
                    const uint32_t idx = (p * 4);

                    /* We'll use div/mul instead of bit shifts to upscale the 5-bit
                     * color values into 8-bit, for potentially better dynamic range.*/
                    const float scale = (255 / 31.0);

                    (*dstTextures)[i].pixelArray[idx + 0] = (kacTexture->pixels[p].r * scale);
                    (*dstTextures)[i].pixelArray[idx + 1] = (kacTexture->pixels[p].g * scale);
                    (*dstTextures)[i].pixelArray[idx + 2] = (kacTexture->pixels[p].b * scale);
                    (*dstTextures)[i].pixelArray[idx + 3] = (kacTexture->pixels[p].a * 255);
                }

                /* Also keep the 16-bit format around, as ARGB 1555.*/
                (*dstTextures)[i].pixelArray16bit[p] = (kacTexture->pixels[p].a << 15) |
                                                      (kacTexture->pixels[p].r << 10) |
                                                      (kacTexture->pixels[p].g << 5) |
                                                      (kacTexture->pixels[p].b << 0);
            }
        }

        for (i = 0; i < numTriangles; i++)
        {
            struct shiet_polygon_triangle_s shietTriangle;

            /* Assign vertices.*/
            {
                uint32_t v = 0;

                for (v = 0; v < 3; v++)
                {
                    const struct kac_1_0_vertex_coordinates_s *vertex = &kacVertexCoords[kacTriangles[i].vertices[v].vertexCoordinatesIdx];
                    const struct kac_1_0_uv_coordinates_s *uv = &kacUVCoords[kacTriangles[i].vertices[v].uvIdx];
                    const struct kac_1_0_normal_s *normal = &kacNormals[kacTriangles[i].vertices[v].normalIdx];

                    shietTriangle.vertex[v].x = vertex->x;
                    shietTriangle.vertex[v].y = vertex->y;
                    shietTriangle.vertex[v].z = vertex->z;
                    shietTriangle.vertex[v].w = 1;

                    shietTriangle.vertex[v].nx = normal->x;
                    shietTriangle.vertex[v].ny = normal->y;
                    shietTriangle.vertex[v].nz = normal->z;

                    shietTriangle.vertex[v].u = uv->u;
                    shietTriangle.vertex[v].v = uv->v;
                }
            }

            /* Assign materials.*/
            {
                const struct kac_1_0_material_s *material = &kacMaterials[kacTriangles[i].materialIdx];

                /* Use div/mul instead of bit shift to upscale 5-bit color
                 * into 8-bit, for potentially better dynamic range.*/
                const float scale = (255 / 15.0);

                shietTriangle.material.texture = &(*dstTextures)[material->metadata.textureMetadataIdx];

                /* KAC 1.0 polygon colors are in the RGBA 4444 format.*/
                shietTriangle.material.baseColor[0] = (material->color.r * scale);
                shietTriangle.material.baseColor[1] = (material->color.g * scale);
                shietTriangle.material.baseColor[2] = (material->color.b * scale);
                shietTriangle.material.baseColor[3] = (material->color.a * scale);
            }

            shiet_tristack_push_copy(dstTriangles, &shietTriangle);
        }

        RELEASE_TEMPORARY_KAC_BUFFERS;
        return 1;
    }

    RELEASE_TEMPORARY_KAC_BUFFERS;
    return 0;

    #undef RELEASE_TEMPORARY_KAC_BUFFERS
}
