/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x rasterizer for the shiet renderer.
 * 
 */

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <shiet_lib/renderer/rasterizer/glide3/rasterizer_glide3.h>
#include <shiet/polygon/triangle/triangle.h>
#include <shiet/polygon/texture.h>

#include <glide/glide.h>

/* Minimum and maximum side length for any given texture.*/
static const unsigned MAX_TEXTURE_SIZE = 256;
static const unsigned MIN_TEXTURE_SIZE = 2;

/* The address in texture memory (assumed for TMU0) when the next texture can be
 * placed. This will be increased as more textures are loaded.*/
static FxU32 CURRENT_TEXTURE_ADDRESS = 0;

/* The vertex structure we want Glide to use. This matches the parameters and
 * their order we give via grVertexLayout() in shiet_rasterizer_glide3__initialize().*/
struct glide_vert_s
{
    FxFloat x, y;
    FxFloat q;
    FxFloat r, g, b;
    FxFloat s, t;
};

void shiet_rasterizer_glide3__initialize(void)
{
    grColorMask(FXTRUE, FXFALSE);

    /* Vertex data layout. Matches with the glide_vert_s struct.*/
    grVertexLayout(GR_PARAM_XY, 0, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Q, 8, GR_PARAM_ENABLE); 
    grVertexLayout(GR_PARAM_RGB, 12, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_ST0, 24, GR_PARAM_ENABLE);

    /* Depth testing.*/
    grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
    grDepthBufferFunction(GR_CMP_LESS); 
    grDepthMask(FXTRUE);

    /* Texturing.*/
    {
        grTexClampMode(GR_TMU0,
                       GR_TEXTURECLAMP_WRAP,
                       GR_TEXTURECLAMP_WRAP);

        /* Set up for decal texturing.*/
        grTexCombine(GR_TMU0,
                     GR_COMBINE_FUNCTION_LOCAL,
                     GR_COMBINE_FACTOR_NONE,
                     GR_COMBINE_FUNCTION_LOCAL,
                     GR_COMBINE_FACTOR_NONE,
                     FXFALSE,
                     FXFALSE);

        grTexLodBiasValue(GR_TMU0, 0.5);
    }

    CURRENT_TEXTURE_ADDRESS = grTexMinAddress(GR_TMU0);

    return;
}

void shiet_rasterizer_glide3__clear_frame(void)
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

static GrTexInfo generate_glide_texture_info(const struct shiet_polygon_texture_s *const texture)
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

void shiet_rasterizer_glide3__upload_texture(struct shiet_polygon_texture_s *const texture)
{
    uint32_t m = 0;
    FxU32 textureSize = 0;
    GrTexInfo texInfo = {0};

    if (!texture)
    {
        return;
    }

    texInfo = generate_glide_texture_info(texture);
    textureSize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &texInfo);

    assert(((CURRENT_TEXTURE_ADDRESS + textureSize) <= grTexMaxAddress(GR_TMU0)) &&
           "Glide 3.x renderer: Not enough texture memory to store the given texture.");

    texture->apiId = CURRENT_TEXTURE_ADDRESS;

    if (texture->numMipLevels > 1)
    {
        for (m = 0; m < texture->numMipLevels; m++)
        {
            grTexDownloadMipMapLevel(GR_TMU0,
                                     CURRENT_TEXTURE_ADDRESS,
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
        grTexDownloadMipMap(GR_TMU0, CURRENT_TEXTURE_ADDRESS, GR_MIPMAPLEVELMASK_BOTH, &texInfo);
    }

    CURRENT_TEXTURE_ADDRESS += textureSize;

    return;
}

void shiet_rasterizer_glide3__update_texture(const struct shiet_polygon_texture_s *const texture)
{
    GrTexInfo texInfo;
    
    if (!texture)
    {
        return;
    }

    texInfo = generate_glide_texture_info(texture);

    grTexDownloadMipMap(GR_TMU0, texture->apiId, GR_MIPMAPLEVELMASK_BOTH, &texInfo);

    return;
}


void shiet_rasterizer_glide3__draw_triangles(const struct shiet_polygon_triangle_s *const triangles,
                                             const unsigned numTriangles)
{
    unsigned i = 0, v = 0;

    for (i = 0; i < numTriangles; i++)
    {
        struct glide_vert_s glideVertex[3];
        const struct shiet_polygon_texture_s *const texture = triangles[i].material.texture;

        for (v = 0; v < 3; v++)
        {
            glideVertex[v].r = triangles[i].material.baseColor[0];
            glideVertex[v].g = triangles[i].material.baseColor[1];
            glideVertex[v].b = triangles[i].material.baseColor[2];

            glideVertex[v].x = triangles[i].vertex[v].x;
            glideVertex[v].y = triangles[i].vertex[v].y;
            glideVertex[v].q = (1 / triangles[i].vertex[v].w);
        }

        /* TODO: Reduce state-switching (e.g. grTexFilterMode()) - batch the
         * polygons by material etc.*/
    
        /* Set the rendering mode based on whether the triangle has a texture
         * or is solid-filled.*/
        if (!texture)
        {
            grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                           GR_COMBINE_FACTOR_NONE,
                           GR_COMBINE_LOCAL_ITERATED,
                           GR_COMBINE_OTHER_NONE,
                           FXFALSE);
        }
        else
        {
            GrTexInfo texInfo = generate_glide_texture_info(texture);

            for (v = 0; v < 3; v++)
            {
                glideVertex[v].s = ((triangles[i].vertex[v].u / triangles[i].vertex[v].w) * 256);
                glideVertex[v].t = ((triangles[i].vertex[v].v / triangles[i].vertex[v].w) * 256);
            }

            if (texture->filtering == SHIET_TEXTURE_FILTER_LINEAR)
            {
                grTexFilterMode(GR_TMU0,
                                GR_TEXTUREFILTER_BILINEAR,
                                GR_TEXTUREFILTER_BILINEAR);
            }
            else
            {
                grTexFilterMode(GR_TMU0,
                                GR_TEXTUREFILTER_POINT_SAMPLED,
                                GR_TEXTUREFILTER_POINT_SAMPLED);
            }

            grTexMipMapMode(GR_TMU0,
                            ((texture->numMipLevels > 1)? GR_MIPMAP_NEAREST : GR_MIPMAP_DISABLE),
                            FXTRUE);

            grTexSource(GR_TMU0, texture->apiId, GR_MIPMAPLEVELMASK_BOTH, &texInfo);

            grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
                           GR_COMBINE_FACTOR_LOCAL,
                           GR_COMBINE_LOCAL_ITERATED,
                           GR_COMBINE_OTHER_TEXTURE,
                           FXFALSE);
        }

        grDrawTriangle(&glideVertex[0], &glideVertex[1], &glideVertex[2]);
    }

    return;
}
