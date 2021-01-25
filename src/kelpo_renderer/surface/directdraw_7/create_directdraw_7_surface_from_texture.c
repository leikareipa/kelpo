/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo renderer
 * 
 * Creates a DirectDraw 7 surface out of a Kelpo texture's data.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
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

/* Used in conjunction with IDirect3DDevice_EnumTextureFormats(); matches an ARGB
 * 1555 pixel format, and if found, copies it into 'dest'.*/
static HRESULT CALLBACK texture_search_callback_argb_1555(DDPIXELFORMAT* pddpf, VOID* dest)
{
    if ((pddpf->dwRGBAlphaBitMask != 0x8000) ||
        (pddpf->dwRBitMask != 0x7c00) ||
        (pddpf->dwGBitMask != 0x3e0) ||
        (pddpf->dwBBitMask != 0x1f))
    {
        return DDENUMRET_OK;
    }

    memcpy((DDPIXELFORMAT*)dest, pddpf, sizeof(DDPIXELFORMAT));

    return DDENUMRET_CANCEL;
}

LPDIRECTDRAWSURFACE7 kelpo_create_directdraw_7_surface_from_texture(const struct kelpo_polygon_texture_s *const texture,
                                                                    LPDIRECT3DDEVICE7 d3dDevice)
{
    uint32_t m = 0;
    HRESULT hr = 0;
    LPDIRECTDRAWSURFACE7 d3dTexture = NULL;
    LPDIRECTDRAWSURFACE7 mipSurface = NULL;

    assert(d3dDevice && "Unknown Direct3D device.");

    /* We require that the Direct3D device is a hardware one, so assert to make sure.*/
    {
        D3DDEVICEDESC7 deviceDescription;
        
        if (FAILED(hr = IDirect3DDevice7_GetCaps(d3dDevice, &deviceDescription)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_D3D_COULDNT_QUERY_DEVICE_CAPS);
            return NULL;
        }

        #ifdef __cplusplus
            assert(((deviceDescription.deviceGUID == IID_IDirect3DHALDevice) ||
                    (deviceDescription.deviceGUID == IID_IDirect3DTnLHalDevice)) &&
        #else
            assert(((deviceDescription.deviceGUID == &IID_IDirect3DHALDevice) ||
                    (deviceDescription.deviceGUID == &IID_IDirect3DTnLHalDevice)) &&
        #endif
                   "Expected a hardware device.");
    }

    /* Create the texture's DirectDraw surface.*/
    {
        LPDIRECTDRAWSURFACE7 renderTarget = NULL;
        LPDIRECTDRAW7 directDrawInterface = NULL;
        DDSURFACEDESC2 surfaceDescription;

        memset(&surfaceDescription, 0, sizeof(DDSURFACEDESC2));
        surfaceDescription.dwSize = sizeof(DDSURFACEDESC2);
        surfaceDescription.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT| DDSD_TEXTURESTAGE);
        surfaceDescription.ddsCaps.dwCaps = (DDSCAPS_TEXTURE | ((texture->numMipLevels <= 1)? 0 : (DDSCAPS_MIPMAP | DDSCAPS_COMPLEX)));
        surfaceDescription.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
        surfaceDescription.dwMipMapCount = 0; /* Kelpo textures with mipmapping are expected to have levels down to 1 x 1.*/
        surfaceDescription.dwWidth = texture->width;
        surfaceDescription.dwHeight = texture->height;
        IDirect3DDevice_EnumTextureFormats(d3dDevice, texture_search_callback_argb_1555, &surfaceDescription.ddpfPixelFormat);

        IDirect3DDevice7_GetRenderTarget(d3dDevice, &renderTarget);
        IDirectDrawSurface7_GetDDInterface(renderTarget, (VOID**)&directDrawInterface);
        IDirectDrawSurface7_Release(renderTarget);

        if (FAILED(hr = IDirectDraw7_CreateSurface(directDrawInterface, &surfaceDescription, &d3dTexture, NULL)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_DDRAW_COULDNT_CREATE_SURFACE);
            IDirectDraw7_Release(directDrawInterface);
            return NULL;
        }

        IDirectDraw7_Release(directDrawInterface);
    }

    /* Copy the Kelpo texture's pixel data into the Direct3D texture.*/
    mipSurface = d3dTexture;
    for (m = 0; m < texture->numMipLevels; m++)
    {
        DDSURFACEDESC2 mipSurfaceDesc;
        const unsigned mipLevelSideLength = (texture->width / pow(2, m)); /* Kelpo textures are expected to be square.*/
        
        /* Copy the texture's mip level pixel data into the DirectDraw surface.*/
        {
            /* Expected pixel color format: ARGB 1555 (16 bits).*/
            const unsigned bytesPerPixel = 2;
            uint16_t *dstPixels = NULL;

            mipSurfaceDesc.dwSize = sizeof(mipSurfaceDesc);

            if (FAILED(hr = IDirectDrawSurface7_Lock(mipSurface, NULL, &mipSurfaceDesc, DDLOCK_WAIT, NULL)))
            {
                fprintf(stderr, "DirectDraw error 0x%x\n", hr);
                kelpo_error(KELPOERR_DDRAW_COULDNT_LOCK_SURFACE);
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

            if (FAILED(hr = IDirectDrawSurface7_Unlock(mipSurface, NULL)))
            {
                fprintf(stderr, "DirectDraw error 0x%x\n", hr);
                kelpo_error(KELPOERR_DDRAW_COULDNT_UNLOCK_SURFACE);
                return NULL;
            }
        }

        /* Move onto the next surface in the mip chain.*/
        if ((texture->numMipLevels > 1) &&
            (m < (texture->numMipLevels - 1)))
        {
            DDSCAPS2 ddsCaps;

            ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_MIPMAP);
            ddsCaps.dwCaps2 = 0;
            ddsCaps.dwCaps3 = 0;
            ddsCaps.dwCaps4 = 0;

            if (SUCCEEDED(hr = IDirectDrawSurface7_GetAttachedSurface(mipSurface, &ddsCaps, &mipSurface)))
            {
                IDirectDrawSurface7_Release(mipSurface);
            }
            else
            {
                fprintf(stderr, "DirectDraw error 0x%x\n", hr);
                kelpo_error(KELPOERR_DDRAW_SURFACE_NOT_AVAILABLE);
                return NULL;
            }
        }
    }

    return d3dTexture;
}
