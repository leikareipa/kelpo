/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo renderer
 * 
 * Creates a DirectDraw 5 surface out of a Kelpo texture's data.
 * 
 * Note: The DirectX 5 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <kelpo_interface/polygon/texture.h>
#include <kelpo_interface/error.h>

#include <windows.h>
#include <d3d.h>

/* A DirectDraw 5 interface expected to have been created before calling
 * any of the functions in this unit.*/
extern LPDIRECTDRAW DIRECTDRAW_5;

/* Used in conjunction with IDirect3DDevice_EnumTextureFormats(); matches an ARGB
 * 1555 pixel format, and if found, copies it into 'dest'.*/
static HRESULT CALLBACK texture_search_callback_argb_1555(LPDDSURFACEDESC pddpf, VOID *dest)
{
    if ((pddpf->ddpfPixelFormat.dwRGBAlphaBitMask != 0x8000) ||
        (pddpf->ddpfPixelFormat.dwRBitMask != 0x7c00) ||
        (pddpf->ddpfPixelFormat.dwGBitMask != 0x3e0) ||
        (pddpf->ddpfPixelFormat.dwBBitMask != 0x1f))
    {
        return DDENUMRET_OK;
    }

    memcpy((DDPIXELFORMAT*)dest, &pddpf->ddpfPixelFormat, sizeof(DDPIXELFORMAT));

    return DDENUMRET_CANCEL;
}

LPDIRECTDRAWSURFACE kelpo_create_directdraw_5_surface_from_texture(const struct kelpo_polygon_texture_s *const texture,
                                                                   LPDIRECT3DDEVICE2 d3dDevice)
{
    uint32_t m = 0;
    HRESULT hr = 0;
    LPDIRECTDRAWSURFACE d3dTexture = NULL;
    LPDIRECTDRAWSURFACE mipSurface = NULL;

    assert(DIRECTDRAW_5 && "The DirectDraw 5 interface has not been initialized.");

    assert(d3dDevice && "Unknown Direct3D device.");

    /* TODO: Test to make sure the D3D device is a hardware one (IID_IDirect3DHALDevice).*/

    /* Create the texture's DirectDraw surface.*/
    {
        DDSURFACEDESC surfaceDescription = {0};

        memset(&surfaceDescription, 0, sizeof(DDSURFACEDESC));
        surfaceDescription.dwSize = sizeof(DDSURFACEDESC);
        surfaceDescription.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
        surfaceDescription.ddsCaps.dwCaps = (DDSCAPS_TEXTURE | ((texture->numMipLevels <= 1)? 0 : (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX)));
        surfaceDescription.dwMipMapCount = 0; /* Kelpo textures with mipmapping are expected to have levels down to 1 x 1.*/
        surfaceDescription.dwWidth = texture->width;
        surfaceDescription.dwHeight = texture->height;
        IDirect3DDevice_EnumTextureFormats(d3dDevice, texture_search_callback_argb_1555, &surfaceDescription.ddpfPixelFormat);

        if (FAILED(hr = IDirectDraw2_CreateSurface(DIRECTDRAW_5, &surfaceDescription, &d3dTexture, NULL)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_API_CALL_FAILED);
            return NULL;
        }
    }

    /* Copy the Kelpo texture's pixel data into the Direct3D texture.*/
    mipSurface = d3dTexture;
    for (m = 0; m < texture->numMipLevels; m++)
    {
        DDSURFACEDESC mipSurfaceDesc = {0};
        const unsigned mipLevelSideLength = (texture->width / pow(2, m)); /* Kelpo textures are expected to be square.*/

        memset(&mipSurfaceDesc, 0, sizeof(mipSurfaceDesc));
        mipSurfaceDesc.dwSize = sizeof(mipSurfaceDesc);
        
        /* Copy the texture's mip level pixel data into the DirectDraw surface.*/
        {
            /* Expected pixel color format: ARGB 1555 (16 bits).*/
            const unsigned bytesPerPixel = 2;
            uint16_t *dstPixels = NULL;

            if (FAILED(hr = IDirectDrawSurface_Lock(mipSurface, NULL, &mipSurfaceDesc, (DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR), NULL)))
            {
                fprintf(stderr, "DirectDraw error 0x%x\n", hr);
                kelpo_error(KELPOERR_API_CALL_FAILED);
                return NULL;
            }

            assert(((mipSurfaceDesc.dwWidth == mipLevelSideLength) &&
                    (mipSurfaceDesc.dwHeight == mipLevelSideLength)) &&
                    "Invalid texture surface dimensions.");

            assert(((mipSurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask == 0x8000) &&
                    (mipSurfaceDesc.ddpfPixelFormat.dwRBitMask == 0x7c00) &&
                    (mipSurfaceDesc.ddpfPixelFormat.dwGBitMask == 0x3e0) &&
                    (mipSurfaceDesc.ddpfPixelFormat.dwBBitMask == 0x1f)) &&
                    "Invalid pixel format for a texture surface. Expected ARGB 1555.");

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

            if (FAILED(hr = IDirectDrawSurface_Unlock(mipSurface, NULL)))
            {
                fprintf(stderr, "DirectDraw error 0x%x\n", hr);
                kelpo_error(KELPOERR_API_CALL_FAILED);
                return NULL;
            }
        }

        /* Move onto the next surface in the mip chain.*/
        if (m < (texture->numMipLevels - 1))
        {
            DDSCAPS ddsCaps = {0};

            ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_MIPMAP);

            if (FAILED(hr = IDirectDrawSurface_GetAttachedSurface(mipSurface, &ddsCaps, &mipSurface)))
            {
                fprintf(stderr, "DirectDraw error 0x%x\n", hr);
                kelpo_error(KELPOERR_API_CALL_FAILED);
                return NULL;
            }
        }
    }

    return d3dTexture;
}
