/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Loads a KAC 1.0 mesh into a shiet-compatible format.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <shiet/polygon/triangle/triangle.h>
#include "kac/import_kac_1_0.h"
#include "load_kac_1_0_mesh.h"

uint32_t shiet_load_kac10_mesh(const char *const kacFilename,
                               struct shiet_polygon_triangle_s **dstTriangles, uint32_t *numTriangles,
                               struct shiet_polygon_texture_s **dstTextures, uint32_t *numTextures)
{
    struct kac_1_0_vertex_coordinates_s *kacVertexCoords = NULL;
    struct kac_1_0_uv_coordinates_s *kacUVCoords = NULL;
    struct kac_1_0_material_s *kacMaterials = NULL;
    struct kac_1_0_triangle_s *kacTriangles = NULL;
    struct kac_1_0_texture_s *kacTextures = NULL;
    struct kac_1_0_normal_s *kacNormals = NULL;

    *numTextures = 0;
    *numTriangles = 0;

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
        (*numTriangles = kac10_reader__read_triangles(&kacTriangles)) &&
        (*numTextures = kac10_reader__read_textures(&kacTextures)) &&
        kac10_reader__read_vertex_coordinates(&kacVertexCoords) &&
        kac10_reader__read_uv_coordinates(&kacUVCoords) &&
        kac10_reader__read_materials(&kacMaterials) &&
        kac10_reader__read_normals(&kacNormals) &&
        kac10_reader__close_file())
    {
        uint32_t i = 0;

        *dstTriangles = malloc(*numTriangles * sizeof(struct shiet_polygon_triangle_s));
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
            /*shietTexture->filtering = (material->metadata.hasTextureFiltering
                                        ? SHIET_TEXTURE_FILTER_LINEAR
                                        : SHIET_TEXTURE_FILTER_NEAREST);*/
            (*dstTextures)[i].filtering = SHIET_TEXTURE_FILTER_LINEAR;

            /* 4 color channels per pixel.*/
            (*dstTextures)[i].pixelArray = malloc(numPixelsInTexture * 4);

            for (p = 0; p < numPixelsInTexture; p++)
            {
                /* 4 color channels per pixel.*/
                const uint32_t idx = (p * 4);

                /* Use div/mul instead of bit shift to upscale 5-bit color
                 * into 8-bit, for potentially better dynamic range.*/
                const float scale = (255 / 31.0);

                /* KAC 1.0 texture colors are in the RGBA 5551 format.*/
                (*dstTextures)[i].pixelArray[idx + 0] = (kacTexture->pixels[p].r * scale);
                (*dstTextures)[i].pixelArray[idx + 1] = (kacTexture->pixels[p].g * scale);
                (*dstTextures)[i].pixelArray[idx + 2] = (kacTexture->pixels[p].b * scale);
                (*dstTextures)[i].pixelArray[idx + 3] = (kacTexture->pixels[p].a * 255);
            }
        }

        for (i = 0; i < *numTriangles; i++)
        {
            /* Assign vertices.*/
            {
                uint32_t v = 0;

                for (v = 0; v < 3; v++)
                {
                    const struct kac_1_0_vertex_coordinates_s *vertex = &kacVertexCoords[kacTriangles[i].vertices[v].vertexCoordinatesIdx];
                    const struct kac_1_0_uv_coordinates_s *uv = &kacUVCoords[kacTriangles[i].vertices[v].uvIdx];
                    const struct kac_1_0_normal_s *normal = &kacNormals[kacTriangles[i].vertices[v].normalIdx];

                    (*dstTriangles)[i].vertex[v].x = vertex->x;
                    (*dstTriangles)[i].vertex[v].y = vertex->y;
                    (*dstTriangles)[i].vertex[v].z = vertex->z;
                    (*dstTriangles)[i].vertex[v].w = 1;

                    (*dstTriangles)[i].vertex[v].nx = normal->x;
                    (*dstTriangles)[i].vertex[v].ny = normal->y;
                    (*dstTriangles)[i].vertex[v].nz = normal->z;

                    (*dstTriangles)[i].vertex[v].u = uv->u;
                    (*dstTriangles)[i].vertex[v].v = uv->v;
                }
            }

            /* Assign materials.*/
            {
                const struct kac_1_0_material_s *material = &kacMaterials[kacTriangles[i].materialIdx];

                /* Use div/mul instead of bit shift to upscale 5-bit color
                 * into 8-bit, for potentially better dynamic range.*/
                const float scale = (255 / 15.0);

                (*dstTriangles)[i].material.texture = &(*dstTextures)[material->metadata.textureMetadataIdx];

                /* KAC 1.0 polygon colors are in the RGBA 4444 format.*/
                (*dstTriangles)[i].material.baseColor[0] = (material->color.r * scale);
                (*dstTriangles)[i].material.baseColor[1] = (material->color.g * scale);
                (*dstTriangles)[i].material.baseColor[2] = (material->color.b * scale);
                (*dstTriangles)[i].material.baseColor[3] = (material->color.a * scale);
            }
        }

        RELEASE_TEMPORARY_KAC_BUFFERS;
        return (*numTriangles >= 1);
    }
    else
    {
        RELEASE_TEMPORARY_KAC_BUFFERS;
        return 0;
    }

    #undef RELEASE_TEMPORARY_KAC_BUFFERS
}
