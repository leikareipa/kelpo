/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Direct3D 7 rasterizer for the Kelpo renderer.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_renderer/surface/directdraw_7/create_directdraw_7_surface_from_texture.h>
#include <kelpo_renderer/surface/direct3d_7/surface_direct3d_7.h>
#include <kelpo_renderer/rasterizer/direct3d_7/rasterizer_direct3d_7.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/polygon/texture.h>
#include <kelpo_interface/error.h>

#include <windows.h>
#include <d3d.h>

/* For keeping track of where in texture memory textures have been uploaded.
 * Stack elements will be of type LPDIRECTDRAWSURFACE7, which represents a
 * pointer to the texture's DirectDraw 7 surface.*/
static struct kelpoa_generic_stack_s *UPLOADED_TEXTURES;

/* For temporary storage of vertices during rendering. Stack elements will be
 * of type D3DTLVERTEX.*/
static struct kelpoa_generic_stack_s *D3D7_VERTEX_CACHE;

extern LPDIRECT3DDEVICE7 D3DDEVICE_7;

int kelpo_rasterizer_direct3d_7__initialize(void)
{
    assert(D3DDEVICE_7 &&
           "Attempting to initialize the rasterizer before the render device has been created.");

    UPLOADED_TEXTURES = kelpoa_generic_stack__create(10, sizeof(LPDIRECTDRAWSURFACE7));
    D3D7_VERTEX_CACHE = kelpoa_generic_stack__create(1000, sizeof(D3DTLVERTEX));

    assert((UPLOADED_TEXTURES && D3D7_VERTEX_CACHE) &&
           "Failed to create data stacks.");

    IDirect3DDevice7_SetTexture(D3DDEVICE_7, 0, NULL);
    
    IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
    IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0, D3DTSS_MAGFILTER, D3DTFN_LINEAR);

    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_ZENABLE, TRUE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_ZWRITEENABLE, TRUE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_CLIPPING, FALSE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_LIGHTING, FALSE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_AMBIENT, ~0u);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_ALPHAREF, 127);

    return 1;
}

int kelpo_rasterizer_direct3d_7__release(void)
{
    assert((UPLOADED_TEXTURES && D3D7_VERTEX_CACHE) &&
           "The data stacks haven't been initialized.");
           
    kelpoa_generic_stack__free(UPLOADED_TEXTURES);
    kelpoa_generic_stack__free(D3D7_VERTEX_CACHE);

    return 1;
}

int kelpo_rasterizer_direct3d_7__clear_frame(void)
{
    HRESULT hr = 0;

    if (FAILED(hr = IDirect3DDevice7_Clear(D3DDEVICE_7, 0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), 0, 1, 0)))
    {
        fprintf(stderr, "Direct3D error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    return 1;
}

int kelpo_rasterizer_direct3d_7__upload_texture(struct kelpo_polygon_texture_s *const texture)
{
    assert(texture && "Attempting to upload a NULL texture");

    assert(UPLOADED_TEXTURES && "The texture stack hasn't been initialized.");

    LPDIRECTDRAWSURFACE7 d3dTexture = kelpo_create_directdraw_7_surface_from_texture(texture, D3DDEVICE_7);

    if (!d3dTexture)
    {
        return 0;
    }

    texture->apiId = (uint32_t)d3dTexture;
    kelpoa_generic_stack__push_copy(UPLOADED_TEXTURES, &d3dTexture);

    return 1;
}

int kelpo_rasterizer_direct3d_7__update_texture(struct kelpo_polygon_texture_s *const texture)
{
    unsigned m = 0;
    HRESULT hr = 0;
    LPDIRECTDRAWSURFACE7 textureSurface = (LPDIRECTDRAWSURFACE7)texture->apiId;
    LPDIRECTDRAWSURFACE7 mipSurface = textureSurface;

    /* Verify that the new texture's data is compatible with the existing surface.*/
    {
        DDSURFACEDESC2 textureSurfaceDesc = {0};

        textureSurfaceDesc.dwSize = sizeof(textureSurfaceDesc);

        if (FAILED(hr = IDirectDrawSurface7_GetSurfaceDesc(textureSurface, &textureSurfaceDesc)))
        {
            fprintf(stderr, "Direct3D error 0x%x\n", hr);
            kelpo_error(KELPOERR_API_CALL_FAILED);
            return 0;
        }

        assert(((textureSurfaceDesc.dwWidth == texture->width) &&
                (textureSurfaceDesc.dwHeight == texture->height)) &&
               "The dimensions of an existing texture cannot be modified.");
    }

    /* Update the texture's pixel data on all mip levels.*/
    for (m = 0; m < texture->numMipLevels; m++)
    {
        DDSURFACEDESC2 mipSurfaceDesc = {0};
        const unsigned mipLevelSideLength = (texture->width / pow(2, m)); /* Kelpo textures are expected to be square.*/
        
        /* Copy the texture's mip level pixel data into the DirectDraw surface.*/
        {
            /* Expected pixel color format: ARGB 1555 (16 bits).*/
            const unsigned bytesPerPixel = 2;
            uint16_t *dstPixels = NULL;

            mipSurfaceDesc.dwSize = sizeof(mipSurfaceDesc);

            if (FAILED(hr = IDirectDrawSurface7_Lock(mipSurface, NULL, &mipSurfaceDesc, DDLOCK_WAIT, NULL)))
            {
                fprintf(stderr, "Direct3D error 0x%x\n", hr);
                kelpo_error(KELPOERR_API_CALL_FAILED);
                return 0;
            }

            assert(((mipSurfaceDesc.dwWidth == mipLevelSideLength) &&
                    (mipSurfaceDesc.dwHeight == mipLevelSideLength)) &&
                    "Invalid mip level surface dimensions.");

            assert(((mipSurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask == 0x8000) &&
                    (mipSurfaceDesc.ddpfPixelFormat.dwRBitMask == 0x7c00) &&
                    (mipSurfaceDesc.ddpfPixelFormat.dwGBitMask == 0x3e0) &&
                    (mipSurfaceDesc.ddpfPixelFormat.dwBBitMask == 0x1f)) &&
                    "Invalid pixel format for a mip level. Expected ARGB 1555.");

            dstPixels = (uint16_t*)mipSurfaceDesc.lpSurface;

            /* If the surface's pitch indicates no padding bytes are needed, we
             * can copy the pixel data directly.*/
            if (mipSurfaceDesc.lPitch == (mipLevelSideLength * bytesPerPixel))
            {
                memcpy(dstPixels, texture->mipLevel[m], (mipLevelSideLength * mipLevelSideLength * bytesPerPixel));
            }
            /* Otherwise, each horizontal line needs to be padded to fit the pitch.*/
            else
            {
                unsigned q = 0;

                for (q = 0; q < mipLevelSideLength /*texture height*/; q++)
                {
                    memcpy(dstPixels, &texture->mipLevel[m][q * mipLevelSideLength], (mipLevelSideLength * bytesPerPixel));
                    dstPixels += (mipSurfaceDesc.lPitch / bytesPerPixel);
                }
            }

            if (FAILED(hr = IDirectDrawSurface7_Unlock(mipSurface, NULL)))
            {
                fprintf(stderr, "Direct3D error 0x%x\n", hr);
                kelpo_error(KELPOERR_API_CALL_FAILED);
                return 0;
            }
        }

        /* Move onto the next surface in the mip chain.*/
        if ((texture->numMipLevels > 1) &&
            (m < (texture->numMipLevels - 1)))
        {
            DDSCAPS2 ddsCaps = {0};

            ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_MIPMAP);

            if (FAILED(hr = IDirectDrawSurface7_GetAttachedSurface(mipSurface, &ddsCaps, &mipSurface)))
            {
                fprintf(stderr, "Direct3D error 0x%x\n", hr);
                kelpo_error(KELPOERR_API_CALL_FAILED);
                return 0;
            }
        }
    }
    
    return 1;
}

int kelpo_rasterizer_direct3d_7__unload_textures(void)
{
    HRESULT hr = 0;
    unsigned i = 0, m = 0;  

    assert(UPLOADED_TEXTURES && "The texture stack hasn't been initialized.");

    IDirect3DDevice7_SetTexture(D3DDEVICE_7, 0, NULL);

    for (i = 0; i < UPLOADED_TEXTURES->count; i++)
    {
        LPDIRECTDRAWSURFACE7 uploadedTexture = ((LPDIRECTDRAWSURFACE7*)UPLOADED_TEXTURES->data)[i];
        LPDIRECTDRAWSURFACE7 mipSurface = uploadedTexture;
        DDSURFACEDESC2 surfaceDesc = {0};
        
        surfaceDesc.dwSize = sizeof(surfaceDesc);

        if (FAILED(hr = IDirectDrawSurface7_GetSurfaceDesc(uploadedTexture, &surfaceDesc)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_API_CALL_FAILED);
            return 0;
        }

        if (surfaceDesc.dwMipMapCount)
        {
            for (m = 0; m < surfaceDesc.dwMipMapCount; m++)
            {
                DDSCAPS2 ddsCaps = {0};
                ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_MIPMAP);

                if (SUCCEEDED(IDirectDrawSurface7_GetAttachedSurface(mipSurface, &ddsCaps, &mipSurface)))
                {
                    IDirectDrawSurface7_Release(mipSurface);
                }
            }
        }
        else
        {
            IDirectDrawSurface7_Release(uploadedTexture);
        }
        
    }

    kelpoa_generic_stack__clear(UPLOADED_TEXTURES);

    return 1;
}

int kelpo_rasterizer_direct3d_7__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
                                                const unsigned numTriangles)
{
    assert(D3D7_VERTEX_CACHE && "The vertex stack hasn't been initialized.");

    HRESULT hr = 0;
    unsigned numTrianglesProcessed = 0;
    unsigned numTrianglesInBatch = 0;
    const struct kelpo_polygon_triangle_s *triangle = triangles;

    if ((3 * numTriangles) > D3D7_VERTEX_CACHE->capacity)
    {
        kelpoa_generic_stack__grow(D3D7_VERTEX_CACHE, (3 * numTriangles));
    }

    if (FAILED(hr = IDirect3DDevice7_BeginScene(D3DDEVICE_7)))
    {
        fprintf(stderr, "Direct3D error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    /* Render the triangles in batches. Each batch consists of consecutive
     * triangles that share a texture.*/
    while (1)
    {
        D3DTLVERTEX *const vertexCache = (D3DTLVERTEX*)D3D7_VERTEX_CACHE->data;
        
        /* Add the current triangle into the batch.*/
        {
            unsigned v = 0;

            for (v = 0; v < 3; v++)
            {
                const struct kelpo_polygon_vertex_s *const srcVertex = &triangle->vertex[v];
                D3DTLVERTEX *const dstVertex = &vertexCache[(numTrianglesInBatch * 3) + v];

                memset(dstVertex, 0, sizeof(D3DTLVERTEX));

                dstVertex->sx = srcVertex->x;
                dstVertex->sy = srcVertex->y;
                dstVertex->sz = srcVertex->z;
                dstVertex->rhw = srcVertex->w;
                dstVertex->tu = srcVertex->u;
                dstVertex->tv = srcVertex->v;
                dstVertex->color = RGBA_MAKE(srcVertex->r,
                                             srcVertex->g,
                                             srcVertex->b,
                                             srcVertex->a);
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
                const unsigned numVerts = (3 * numTrianglesInBatch);

                if (!hasTexture)
                {
                    IDirect3DDevice7_SetTexture(D3DDEVICE_7, 0, NULL);
                }
                else
                {
                    const int mipmapEnabled = ((triangle->texture->numMipLevels > 1) &&
                                               !triangle->texture->flags.noMipmapping);
                    const int mipmapFilter = (triangle->texture->flags.noFiltering? D3DTFP_POINT : D3DTFP_LINEAR);

                    IDirect3DDevice7_SetTexture(D3DDEVICE_7, 0, (LPDIRECTDRAWSURFACE7)triangle->texture->apiId);

                    IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0,
                                                          D3DTSS_MIPFILTER,
                                                          (mipmapEnabled? mipmapFilter : D3DTFP_NONE));

                    IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0,
                                                          D3DTSS_MINFILTER,
                                                          (triangle->texture->flags.noFiltering? D3DTFN_POINT : D3DTFN_LINEAR));

                    IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0,
                                                          D3DTSS_MAGFILTER,
                                                          (triangle->texture->flags.noFiltering? D3DTFN_POINT : D3DTFN_LINEAR));
                }

                IDirect3DDevice7_DrawPrimitive(D3DDEVICE_7,
                                               D3DPT_TRIANGLELIST,
                                               D3DFVF_TLVERTEX,
                                               vertexCache, numVerts,
                                               NULL);

                if (isEndOfTriangles)
                {
                    break;
                }

                numTrianglesInBatch = 0;
            }
        }

        triangle++;
    }

    if (FAILED(hr = IDirect3DDevice7_EndScene(D3DDEVICE_7)))
    {
        fprintf(stderr, "Direct3D error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }
    
    return 1;
}
