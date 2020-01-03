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
    FxFloat w;
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
    grTexClampMode(GR_TMU0,
                   GR_TEXTURECLAMP_CLAMP,
                   GR_TEXTURECLAMP_CLAMP);
    grTexCombine(GR_TMU0,
                 GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 FXFALSE,
                 FXFALSE); /* Set up for decal texturing.*/
    grTexMipMapMode(GR_TMU0, GR_MIPMAP_DISABLE, FXFALSE);

    CURRENT_TEXTURE_ADDRESS = grTexMinAddress(GR_TMU0);

    return;
}

void shiet_rasterizer_glide3__clear_frame(void)
{
    grBufferClear(0, 0, ~0u);

    return;
}

static GrTexInfo generate_glide_texture_info(const struct shiet_polygon_texture_s *const texture)
{
    GrTexInfo info;

    assert(texture &&
           "Glide 3.x renderer: Received a NULL texture.");

    assert((texture->width == texture->height) &&
           "Glide 3.x renderer: The given texture is not square.");

    assert((texture->width >= MIN_TEXTURE_SIZE) &&
           (texture->width <= MAX_TEXTURE_SIZE) &&
           "Glide 3.x renderer: The given texture's dimensions are out of range.");

    switch (texture->width)
    {
        case 2: info.smallLodLog2 = info.largeLodLog2 = GR_LOD_LOG2_2; break;
        case 4: info.smallLodLog2 = info.largeLodLog2 = GR_LOD_LOG2_4; break;
        case 8: info.smallLodLog2 = info.largeLodLog2 = GR_LOD_LOG2_8; break;
        case 16: info.smallLodLog2 = info.largeLodLog2 = GR_LOD_LOG2_16; break;
        case 32: info.smallLodLog2 = info.largeLodLog2 = GR_LOD_LOG2_32; break;
        case 64: info.smallLodLog2 = info.largeLodLog2 = GR_LOD_LOG2_64; break;
        case 128: info.smallLodLog2 = info.largeLodLog2 = GR_LOD_LOG2_128; break;
        case 256: info.smallLodLog2 = info.largeLodLog2 = GR_LOD_LOG2_256; break;
    }

    info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
    info.format = GR_TEXFMT_ARGB_1555;
    info.data = (FxU16*)texture->pixelArray16bit;

    return info;
}

void shiet_rasterizer_glide3__upload_texture(struct shiet_polygon_texture_s *const texture)
{
    GrTexInfo texInfo;
    FxU32 textureSize; 

    if (!texture)
    {
        return;
    }

    texInfo = generate_glide_texture_info(texture);
    textureSize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &texInfo); 

    assert(((CURRENT_TEXTURE_ADDRESS + textureSize) <= grTexMaxAddress(GR_TMU0)) &&
           "Glide 3.x renderer: Not enough texture memory to store the given texture.");

    grTexDownloadMipMap(GR_TMU0, CURRENT_TEXTURE_ADDRESS, GR_MIPMAPLEVELMASK_BOTH, &texInfo);
    texture->apiId = CURRENT_TEXTURE_ADDRESS;

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
            glideVertex[v].w = (1 / triangles[i].vertex[v].w);

            glideVertex[v].r = triangles[i].material.baseColor[0];
            glideVertex[v].g = triangles[i].material.baseColor[1];
            glideVertex[v].b = triangles[i].material.baseColor[2];

            glideVertex[v].x = triangles[i].vertex[v].x;
            glideVertex[v].y = triangles[i].vertex[v].y;
        }
    
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
            const unsigned glideTexScale = (256 / texture->width);
            GrTexInfo texInfo = generate_glide_texture_info(texture);

            for (v = 0; v < 3; v++)
            {
                glideVertex[v].s = ((triangles[i].vertex[0].u / triangles[i].vertex[0].w) * glideTexScale);
                glideVertex[v].t = ((triangles[i].vertex[0].v / triangles[i].vertex[0].w) * glideTexScale);
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
