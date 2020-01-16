/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Loads a KAC 1.0 mesh into a kelpo-compatible format.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <kelpo_interface/generic_stack.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include "kac/import_kac_1_0.h"
#include "load_kac_1_0_mesh.h"

int kelpo_load_kac10_mesh(const char *const kacFilename,
                          struct kelpo_generic_stack_s *dstTriangles,
                          struct kelpo_polygon_texture_s **dstTextures,
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

    #define FREE_TEMPORARY_KAC_BUFFERS {uint32_t i = 0, m = 0;\
                                        for (i = 0; i < *numTextures; i++)\
                                        {\
                                            for (m = 0; m < kacTextures[i].numMipLevels; m++)\
                                            {\
                                                free(kacTextures[i].mipLevel[m]);\
                                            }\
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

        /* Allocate memory for the destination buffers.*/
        kelpo_generic_stack__grow(dstTriangles, numTriangles);
        *dstTextures = malloc(*numTextures * sizeof(struct kelpo_polygon_texture_s));

        /* Convert the KAC textures into Kelpo's internal format.*/
        for (i = 0; i < *numTextures; i++)
        {
            uint32_t p = 0, m = 0;

            /* The code may rely on bit fields or unallocated pointers being 0,
             * so let's accommodate.*/
            memset(&(*dstTextures)[i], 0, sizeof(struct kelpo_polygon_texture_s));

            (*dstTextures)[i].width = kacTextures[i].metadata.sideLength;
            (*dstTextures)[i].height = kacTextures[i].metadata.sideLength;
            (*dstTextures)[i].numMipLevels = kacTextures[i].numMipLevels;
            (*dstTextures)[i].flags.clamped = kacTextures[i].metadata.clampUV;
            (*dstTextures)[i].flags.noFiltering = !kacTextures[i].metadata.sampleLinearly;

            /* Get the pixels for all levels of mipmapping, starting at level 0 and
             * progressively halving the resolution until we're down to 1 x 1.*/
            for (m = 0; m < kacTextures[i].numMipLevels; m++)
            {
                const uint32_t mipLevelSideLength = ((*dstTextures)[i].width / pow(2, m));
                const uint32_t mipLevelPixelCount = (mipLevelSideLength * mipLevelSideLength);

                /* We should never end up at a mip level smaller than the minumum.
                 * But if we do, it may indicate an incorrect mip level count for
                 * this texture in the KAC data.*/
                if (mipLevelSideLength < KAC_1_0_MIN_TEXTURE_SIDE_LENGTH)
                {
                    goto fail;
                }

                (*dstTextures)[i].mipLevel[m] = malloc(mipLevelPixelCount * sizeof((*dstTextures)[i].mipLevel[m][0]));
                for (p = 0; p < mipLevelPixelCount; p++)
                {
                    (*dstTextures)[i].mipLevel[m][p] = (kacTextures[i].mipLevel[m][p].a << 15) |
                                                       (kacTextures[i].mipLevel[m][p].r << 10) |
                                                       (kacTextures[i].mipLevel[m][p].g << 5)  |
                                                       (kacTextures[i].mipLevel[m][p].b << 0);
                }

            }
        }

        for (i = 0; i < numTriangles; i++)
        {
            struct kelpo_polygon_triangle_s kelpoTriangle;
            uint32_t v = 0;
            const struct kac_1_0_material_s *material = &kacMaterials[kacTriangles[i].materialIdx];

            /* The code may rely on bit fields or the like being initialized to 0,
             * so let's accommodate.*/
            memset(&kelpoTriangle, 0, sizeof(struct kelpo_polygon_triangle_s));

            for (v = 0; v < 3; v++)
            {
                /* We'll use div/mul instead of a bit shift to upscale KAC's 4-bit
                 * polygon colors into 8-bit, for potentially better dynamic range.*/
                const float materialColorScale = (255 / 15.0);

                const struct kac_1_0_vertex_coordinates_s *vertex = &kacVertexCoords[kacTriangles[i].vertices[v].vertexCoordinatesIdx];
                const struct kac_1_0_uv_coordinates_s *uv = &kacUVCoords[kacTriangles[i].vertices[v].uvIdx];
                const struct kac_1_0_normal_s *normal = &kacNormals[kacTriangles[i].vertices[v].normalIdx];

                kelpoTriangle.vertex[v].x = vertex->x;
                kelpoTriangle.vertex[v].y = vertex->y;
                kelpoTriangle.vertex[v].z = vertex->z;
                kelpoTriangle.vertex[v].w = 1;

                kelpoTriangle.vertex[v].nx = normal->x;
                kelpoTriangle.vertex[v].ny = normal->y;
                kelpoTriangle.vertex[v].nz = normal->z;

                kelpoTriangle.vertex[v].u = uv->u;
                kelpoTriangle.vertex[v].v = uv->v;

                kelpoTriangle.vertex[v].r = (material->color.r * materialColorScale);
                kelpoTriangle.vertex[v].g = (material->color.g * materialColorScale);
                kelpoTriangle.vertex[v].b = (material->color.b * materialColorScale);
                kelpoTriangle.vertex[v].a = (material->color.a * materialColorScale);
            }

            if (material->metadata.hasTexture)
            {
                kelpoTriangle.texture = &(*dstTextures)[material->metadata.textureIdx];
            }

            kelpo_generic_stack__push_copy(dstTriangles, &kelpoTriangle);
        }

        FREE_TEMPORARY_KAC_BUFFERS;
        return 1;
    }

    fail:
    FREE_TEMPORARY_KAC_BUFFERS;
    return 0;

    #undef FREE_TEMPORARY_KAC_BUFFERS
}
