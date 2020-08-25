/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Direct3D 5 rasterizer for the Kelpo renderer.
 * 
 * Note: The DirectX 5 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_renderer/surface/directdraw_5/create_directdraw_5_surface_from_texture.h>
#include <kelpo_renderer/surface/direct3d_5/surface_direct3d_5.h>
#include <kelpo_renderer/rasterizer/direct3d_5/rasterizer_direct3d_5.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/polygon/texture.h>

#include <windows.h>
#include <d3d.h>

/* For keeping track of where in texture memory textures have been uploaded.
 * Stack elements will be of type LPDIRECTDRAWSURFACE3, which represents a
 * pointer to the texture's DirectDraw 5 surface.*/
static struct kelpoa_generic_stack_s *UPLOADED_TEXTURES;

extern LPDIRECT3DDEVICE2 D3DDEVICE_5;
extern LPDIRECT3DVIEWPORT2 D3DVIEWPORT_5;
static D3DVIEWPORT SURFACE_VIEWPORT;

void kelpo_rasterizer_direct3d_5__initialize(void)
{
    assert(D3DVIEWPORT_5 && D3DDEVICE_5 &&
           "Direct3D 5: Attempting to initialize the rasterizer before the render device has been created.");

    IDirect3DDevice2_SetRenderState(D3DDEVICE_5, D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
    IDirect3DDevice2_SetRenderState(D3DDEVICE_5, D3DRENDERSTATE_ZENABLE, TRUE);
    IDirect3DDevice2_SetRenderState(D3DDEVICE_5, D3DRENDERSTATE_ZWRITEENABLE, TRUE);
    IDirect3DDevice2_SetRenderState(D3DDEVICE_5, D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
    IDirect3DDevice2_SetRenderState(D3DDEVICE_5, D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);

    memset(&SURFACE_VIEWPORT, 0, sizeof(SURFACE_VIEWPORT));
    SURFACE_VIEWPORT.dwSize = sizeof(SURFACE_VIEWPORT);
    IDirect3DViewport2_GetViewport(D3DVIEWPORT_5, &SURFACE_VIEWPORT);

    UPLOADED_TEXTURES = kelpoa_generic_stack__create(10, sizeof(LPDIRECTDRAWSURFACE3));

    return;
}

void kelpo_rasterizer_direct3d_5__release(void)
{
    kelpoa_generic_stack__free(UPLOADED_TEXTURES);

    return;
}

void kelpo_rasterizer_direct3d_5__clear_frame(void)
{
    D3DRECT screenRect = {0, 0, SURFACE_VIEWPORT.dwWidth, SURFACE_VIEWPORT.dwHeight};

    IDirect3DViewport2_Clear(D3DVIEWPORT_5, 1, &screenRect, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER));

    return;
}

void kelpo_rasterizer_direct3d_5__upload_texture(struct kelpo_polygon_texture_s *const texture)
{
    D3DTEXTUREHANDLE d3dTextureHandle;
    LPDIRECT3DTEXTURE2 textureObject;
    LPDIRECTDRAWSURFACE d3dTexture = kelpo_create_directdraw_5_surface_from_texture(texture, D3DDEVICE_5);

    assert(d3dTexture && "Direct3D 5: Failed to create a Direct3D texture.");

    if (FAILED(IDirectDraw2_QueryInterface(d3dTexture, IID_IDirect3DTexture2, (LPVOID *)&textureObject)))
    {
        fprintf(stderr, "ERROR: Direct3D 5: Failed to upload a texture.");

		return;
    }

	IDirect3DTexture_GetHandle(textureObject, D3DDEVICE_5, &d3dTextureHandle);
	IDirect3DTexture_Release(textureObject);

    texture->apiId = (uint32_t)d3dTextureHandle;
    texture->apiAuxData = d3dTexture;
    kelpoa_generic_stack__push_copy(UPLOADED_TEXTURES, &d3dTexture);

    return;
}

void kelpo_rasterizer_direct3d_5__update_texture(struct kelpo_polygon_texture_s *const texture)
{
    unsigned m = 0;
    LPDIRECTDRAWSURFACE textureSurface = (LPDIRECTDRAWSURFACE)texture->apiAuxData;
    LPDIRECTDRAWSURFACE mipSurface = textureSurface;

    /* Verify that the new texture's data is compatible with the existing surface.*/
    {
        DDSURFACEDESC textureSurfaceDesc;

        textureSurfaceDesc.dwSize = sizeof(textureSurfaceDesc);
        if (FAILED(IDirectDrawSurface3_GetSurfaceDesc(textureSurface, &textureSurfaceDesc)))
        {
            return;
        }

        assert(((textureSurfaceDesc.dwWidth == texture->width) &&
                (textureSurfaceDesc.dwHeight == texture->height)) &&
               "Direct3D 5: The dimensions of an existing texture cannot be modified.");
    }

    /* Update the texture's pixel data on all mip levels.*/
    for (m = 0; m < texture->numMipLevels; m++)
    {
        DDSURFACEDESC mipSurfaceDesc;
        const unsigned mipLevelSideLength = (texture->width / pow(2, m)); /* Kelpo textures are expected to be square.*/
        
        /* Copy the texture's mip level pixel data into the DirectDraw surface.*/
        {
            /* Expected pixel color format: ARGB 1555 (16 bits).*/
            const unsigned bytesPerPixel = 2;
            uint16_t *dstPixels = NULL;

            mipSurfaceDesc.dwSize = sizeof(mipSurfaceDesc);
            if (FAILED(IDirectDrawSurface3_Lock(mipSurface, NULL, &mipSurfaceDesc, DDLOCK_WAIT, NULL)))
            {
                return;
            }

            assert(((mipSurfaceDesc.dwWidth == mipLevelSideLength) &&
                    (mipSurfaceDesc.dwHeight == mipLevelSideLength)) &&
                    "Direct3D 5: Invalid mip level surface dimensions.");

            assert(((mipSurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask == 0x8000) &&
                    (mipSurfaceDesc.ddpfPixelFormat.dwRBitMask == 0x7c00) &&
                    (mipSurfaceDesc.ddpfPixelFormat.dwGBitMask == 0x3e0) &&
                    (mipSurfaceDesc.ddpfPixelFormat.dwBBitMask == 0x1f)) &&
                    "Direct3D 5: Invalid pixel format for a mip level. Expected ARGB 1555.");

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

            if (FAILED(IDirectDrawSurface3_Unlock(mipSurface, NULL)))
            {
                return;
            }
        }

        /* Move onto the next surface in the mip chain.*/
        if ((texture->numMipLevels > 1) &&
            (m < (texture->numMipLevels - 1)))
        {
            DDSCAPS ddsCaps;

            ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_MIPMAP);

            if (SUCCEEDED(IDirectDrawSurface3_GetAttachedSurface(mipSurface, &ddsCaps, &mipSurface)))
            {
                IDirectDrawSurface3_Release(mipSurface);
            }
            else
            {
                return;
            }
        }
    }

    return;
}

void kelpo_rasterizer_direct3d_5__purge_textures(void)
{
    unsigned i = 0;

    IDirect3DDevice2_SetRenderState(D3DDEVICE_5,
                                    D3DRENDERSTATE_TEXTUREHANDLE,
                                    NULL);

    for (i = 0; i < UPLOADED_TEXTURES->count; i++)
    {
        LPDIRECTDRAWSURFACE uploadedTexture = ((LPDIRECTDRAWSURFACE*)UPLOADED_TEXTURES->data)[i];
        IDirectDrawSurface3_Release(uploadedTexture);
    }

    kelpoa_generic_stack__clear(UPLOADED_TEXTURES);

    return;
}

void kelpo_rasterizer_direct3d_5__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
                                                 const unsigned numTriangles)
{
    unsigned i = 0;

    /* We'll keep track of which texture we've rendered with most recently, so
     * we'll know not to re-set the texture pipeline's state if we're rendering
     * with the same texture again. A value of ~0 is assumed to mean "unknown"
     * or "most recent triangle was untextured" - this further assumes that
     * valid texture ids can never have that value. */ 
    unsigned currentTextureApiId = ~0;

    D3DTLVERTEX verts[3];
    memset(verts, 0, (sizeof(D3DTLVERTEX) * 3));

    if (FAILED(IDirect3DDevice2_BeginScene(D3DDEVICE_5)))
    {
        return;
    }

    for (i = 0; i < numTriangles; i++)
    {
        unsigned v = 0;

        for (v = 0; v < 3; v++)
        {
            verts[v].sx = triangles[i].vertex[v].x;
            verts[v].sy = triangles[i].vertex[v].y;
            verts[v].sz = triangles[i].vertex[v].z;
            verts[v].rhw = triangles[i].vertex[v].w;
            verts[v].tu = triangles[i].vertex[v].u;
            verts[v].tv = triangles[i].vertex[v].v;
            verts[v].color = RGBA_MAKE(triangles[i].vertex[v].r,
                                       triangles[i].vertex[v].g,
                                       triangles[i].vertex[v].b,
                                       triangles[i].vertex[v].a);
        }

        /* TODO: Reduce state-switching.*/

        if (triangles[i].texture &&
            (triangles[i].texture->apiId != currentTextureApiId))
        {
            const int mipmapEnabled = (triangles[i].texture->numMipLevels > 1);
            const int mipmapFilter = mipmapEnabled
                                     ? (triangles[i].texture->flags.noFiltering? D3DFILTER_LINEARMIPNEAREST : D3DFILTER_LINEARMIPLINEAR)
                                     : (triangles[i].texture->flags.noFiltering? D3DFILTER_NEAREST : D3DFILTER_LINEAR);

            currentTextureApiId = triangles[i].texture->apiId;

            IDirect3DDevice2_SetRenderState(D3DDEVICE_5,
                                            D3DRENDERSTATE_TEXTUREHANDLE,
                                            (D3DTEXTUREHANDLE)triangles[i].texture->apiId);

            IDirect3DDevice2_SetRenderState(D3DDEVICE_5,
                                            D3DRENDERSTATE_TEXTUREMIN,
                                            mipmapFilter);

            IDirect3DDevice2_SetRenderState(D3DDEVICE_5,
                                            D3DRENDERSTATE_TEXTUREMAG,
                                            mipmapFilter);
        }
        else if (!triangles[i].texture)
        {
            currentTextureApiId = ~0;

            IDirect3DDevice2_SetRenderState(D3DDEVICE_5,
                                            D3DRENDERSTATE_TEXTUREHANDLE,
                                            NULL);
        }

        IDirect3DDevice2_DrawPrimitive(D3DDEVICE_5,
                                       D3DPT_TRIANGLELIST,
                                       D3DVT_TLVERTEX,
                                       verts, 3,
                                       NULL);
    }

    IDirect3DDevice2_EndScene(D3DDEVICE_5);

    return;
}
