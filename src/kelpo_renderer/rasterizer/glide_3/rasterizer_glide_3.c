/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x rasterizer for the Kelpo renderer.
 * 
 */

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <kelpo_renderer/rasterizer/glide_3/rasterizer_glide_3.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/polygon/texture.h>
#include <kelpo_auxiliary/generic_stack.h>

#include <glide/glide.h>

/* For keeping track of where in texture memory textures have been uploaded.
 * Stack elements will be of type FxU32, which represents a pointer to texture
 * memory where the particular texture's data begins. Note that this stores
 * no texture metadata, only the texture memory pointer.*/
static struct kelpoa_generic_stack_s *UPLOADED_TEXTURES;

/* For temporary storage of vertices during rendering. Stack elements will be
 * of type struct glide3_vertex_s.*/
static struct kelpoa_generic_stack_s *GR3_VERTEX_CACHE;

/* Minimum and maximum side length for any given texture.*/
static const unsigned MAX_TEXTURE_SIZE = 256;
static const unsigned MIN_TEXTURE_SIZE = 2;

/* The address in texture memory (assumed for TMU0) where the next texture's
 * data can be uploaded. This will be incremented as textures are loaded in.*/
static FxU32 CURRENT_TEXTURE_ADDRESS = 0;

/* NOTE: This struct would ideally not be padded further by the compiler,
 * since we manually pass its byte-level structure to grVertexLayout().*/
struct glide3_vertex_s
{
    float x, y, q;
    float u, v;
    uint32_t rgba; /* Each of the four color channels takes 1 byte.*/
};

void kelpo_rasterizer_glide_3__initialize(void)
{
    UPLOADED_TEXTURES = kelpoa_generic_stack__create(10, sizeof(FxU32));
    GR3_VERTEX_CACHE = kelpoa_generic_stack__create(1000, sizeof(struct glide3_vertex_s));

    grColorMask(FXTRUE, FXFALSE);

    /* Vertex data layout matches that in the glide3_vertex_s struct.*/
    grVertexLayout(GR_PARAM_XY, 0, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Q, 8, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_ST0, 12, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_PARGB, 20, GR_PARAM_ENABLE);

    grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
    grDepthBufferFunction(GR_CMP_LESS); 
    grDepthMask(FXTRUE);

    grAlphaTestFunction(GR_CMP_GREATER);
    grAlphaTestReferenceValue(0);
    grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                   GR_COMBINE_FACTOR_LOCAL,
                   GR_COMBINE_LOCAL_ITERATED,
                   GR_COMBINE_OTHER_TEXTURE,
                   FXFALSE);

    grTexCombine(GR_TMU0,
                 GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 FXFALSE,
                 FXFALSE);
    grTexLodBiasValue(GR_TMU0, 0.5);

    CURRENT_TEXTURE_ADDRESS = grTexMinAddress(GR_TMU0);  

    return;
}

void kelpo_rasterizer_glide_3__release(void)
{
    kelpoa_generic_stack__free(UPLOADED_TEXTURES);
    kelpoa_generic_stack__free(GR3_VERTEX_CACHE);

    return;
}

void kelpo_rasterizer_glide_3__clear_frame(void)
{
    grBufferClear(0, 0, ~0u);

    return;
}

static GrLOD_t lod_for_size(const unsigned size)
{
    switch (size)
    {
        case 1: return GR_LOD_LOG2_1;
        case 2: return GR_LOD_LOG2_2;
        case 4: return GR_LOD_LOG2_4;
        case 8: return GR_LOD_LOG2_8;
        case 16: return GR_LOD_LOG2_16;
        case 32: return GR_LOD_LOG2_32;
        case 64: return GR_LOD_LOG2_64;
        case 128: return GR_LOD_LOG2_128;
        case 256: return GR_LOD_LOG2_256;
        default: assert(0 && "Unknown size for calculating a LOD value for.");
    }
}

static GrTexInfo generate_glide_texture_info(const struct kelpo_polygon_texture_s *const texture)
{
    GrTexInfo info = {0};

    assert(texture &&
           "Glide 3.x renderer: Received a NULL texture.");

    assert((texture->width == texture->height) &&
           "Glide 3.x renderer: The given texture is not square.");

    assert((texture->width >= MIN_TEXTURE_SIZE) &&
           (texture->width <= MAX_TEXTURE_SIZE) &&
           "Glide 3.x renderer: The given texture's dimensions are out of range.");

    if (texture->numMipLevels > 1)
    {
        info.smallLodLog2 = GR_LOD_LOG2_1;
        info.largeLodLog2 = lod_for_size(texture->width);
        info.data = NULL;
    }
    else
    {
        info.smallLodLog2 = info.largeLodLog2 = lod_for_size(texture->width);
        info.data = texture->mipLevel[0];
    }

    info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
    info.format = GR_TEXFMT_ARGB_1555;

    return info;
}

/* Uploads the given texture's data to the graphics device. The data will be
 * placed in the Glide texture memory address specified by the texture's 'apiId'
 * property, which must be properly initialized prior to calling this function.*/
static void upload_texture_data(struct kelpo_polygon_texture_s *const texture,
                                GrTexInfo *const textureInfo)
{
    if (texture->numMipLevels > 1)
    {
        uint32_t m = 0;

        for (m = 0; m < texture->numMipLevels; m++)
        {
            grTexDownloadMipMapLevel(GR_TMU0,
                                     texture->apiId,
                                     lod_for_size(texture->width / pow(2, m)),
                                     lod_for_size(texture->width),
                                     GR_ASPECT_LOG2_1x1,
                                     GR_TEXFMT_ARGB_1555,
                                     GR_MIPMAPLEVELMASK_BOTH,
                                     texture->mipLevel[m]);
        }
    }
    else
    {
        grTexDownloadMipMap(GR_TMU0, texture->apiId, GR_MIPMAPLEVELMASK_BOTH, textureInfo);
    }

    return;
}

void kelpo_rasterizer_glide_3__upload_texture(struct kelpo_polygon_texture_s *const texture)
{
    FxU32 textureSize = 0;
    GrTexInfo textureInfo = {0};

    if (!texture)
    {
        return;
    }

    textureInfo = generate_glide_texture_info(texture);
    textureSize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &textureInfo);

    assert(((CURRENT_TEXTURE_ADDRESS + textureSize) <= grTexMaxAddress(GR_TMU0)) &&
           "Glide 3.x renderer: Not enough texture memory to store the given texture.");

    texture->apiId = CURRENT_TEXTURE_ADDRESS;
    upload_texture_data(texture, &textureInfo);

    CURRENT_TEXTURE_ADDRESS += textureSize;

    return;
}

void kelpo_rasterizer_glide_3__update_texture(struct kelpo_polygon_texture_s *const texture)
{
    GrTexInfo textureInfo = {0};
    
    if (!texture)
    {
        return;
    }

    textureInfo = generate_glide_texture_info(texture);

    upload_texture_data(texture, &textureInfo);

    return;
}

void kelpo_rasterizer_glide_3__unload_textures(void)
{
    CURRENT_TEXTURE_ADDRESS = grTexMinAddress(GR_TMU0);
    kelpoa_generic_stack__clear(UPLOADED_TEXTURES);

    return;
}

void kelpo_rasterizer_glide_3__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
                                              const unsigned numTriangles)
{
    unsigned numTrianglesProcessed = 0;
    unsigned numTrianglesInBatch = 0;
    const struct kelpo_polygon_triangle_s *triangle = triangles;

    if ((3 * numTriangles) > GR3_VERTEX_CACHE->capacity)
    {
        kelpoa_generic_stack__grow(GR3_VERTEX_CACHE, (3 * numTriangles));
    }

    /* Render the triangles in batches. Each batch consists of consecutive
     * triangles that share a texture.*/
    while (1)
    {
        unsigned v = 0;
        struct glide3_vertex_s *const vertexCache = (struct glide3_vertex_s*)GR3_VERTEX_CACHE->data;
        
        /* Add the current triangle into the batch.*/
        {
            for (v = 0; v < 3; v++)
            {
                const struct kelpo_polygon_vertex_s *const srcVertex = &triangle->vertex[v];
                struct glide3_vertex_s *const dstVertex = &vertexCache[(numTrianglesInBatch * 3) + v];

                dstVertex->x = srcVertex->x;
                dstVertex->y = srcVertex->y;
                dstVertex->q = srcVertex->w;

                dstVertex->u = ((srcVertex->u * srcVertex->w) * 256);
                dstVertex->v = ((srcVertex->v * srcVertex->w) * 256);

                ((char*)&dstVertex->rgba)[0] = srcVertex->b;
                ((char*)&dstVertex->rgba)[1] = srcVertex->g;
                ((char*)&dstVertex->rgba)[2] = srcVertex->r;
                ((char*)&dstVertex->rgba)[3] = srcVertex->a;
            }

            numTrianglesInBatch++;
            numTrianglesProcessed++;
        }

        /* If we're at the end of the current batch, render its triangles and
         * start a new batch.*/
        {
            const int isEndOfTriangles = (numTrianglesProcessed >= numTriangles);

            const int hasTexture = (triangle->texture != NULL);
            const uint32_t currentApiId = (hasTexture? triangle->texture->apiId : 0);
            
            const struct kelpo_polygon_triangle_s *nextTriangle = (isEndOfTriangles? NULL : (triangle + 1));
            const int nextHasTexture = (nextTriangle? (nextTriangle->texture != NULL) : 0);
            const uint32_t nextApiId = (nextHasTexture? nextTriangle->texture->apiId : 0);
            const int isEndOfBatch = (isEndOfTriangles || (nextApiId != currentApiId));

            if (isEndOfBatch)
            {
                if (!hasTexture)
                {
                    grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                                   GR_COMBINE_FACTOR_NONE,
                                   GR_COMBINE_LOCAL_ITERATED,
                                   GR_COMBINE_OTHER_NONE,
                                   FXFALSE);
                }
                else
                {
                    GrTexInfo texInfo = generate_glide_texture_info(triangle->texture);

                    grTexFilterMode(GR_TMU0,
                                    (triangle->texture->flags.noFiltering? GR_TEXTUREFILTER_POINT_SAMPLED : GR_TEXTUREFILTER_BILINEAR),
                                    (triangle->texture->flags.noFiltering? GR_TEXTUREFILTER_POINT_SAMPLED : GR_TEXTUREFILTER_BILINEAR));

                    grTexClampMode(GR_TMU0,
                                   (triangle->texture->flags.clamped? GR_TEXTURECLAMP_CLAMP : GR_TEXTURECLAMP_WRAP),
                                   (triangle->texture->flags.clamped? GR_TEXTURECLAMP_CLAMP : GR_TEXTURECLAMP_WRAP));

                    grTexMipMapMode(GR_TMU0,
                                    ((triangle->texture->numMipLevels > 1)? GR_MIPMAP_NEAREST : GR_MIPMAP_DISABLE),
                                    FXTRUE);

                    grTexSource(GR_TMU0, triangle->texture->apiId, GR_MIPMAPLEVELMASK_BOTH, &texInfo);

                    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                                   GR_COMBINE_FACTOR_LOCAL,
                                   GR_COMBINE_LOCAL_ITERATED,
                                   GR_COMBINE_OTHER_TEXTURE,
                                   FXFALSE);
                }

                for (v = 0; v < numTrianglesInBatch; v++)
                {
                    const unsigned vertexIdx = (v * 3);

                    grDrawTriangle(&vertexCache[vertexIdx+0],
                                   &vertexCache[vertexIdx+1],
                                   &vertexCache[vertexIdx+2]);
                }

                if (isEndOfTriangles)
                {
                    break;
                }

                numTrianglesInBatch = 0;
            }
        }

        triangle++;
    }

    return;
}
