/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Provides a Direct3D-compatible DirectDraw 6 surface for the Kelpo renderer.
 * 
 * Not intended for use as a standalone surface, but as an intermediary for
 * other surfaces that require a DirectDraw 6 backend.
 * 
 * Note: The DirectX 6 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <kelpo_renderer/surface/directdraw_6/enumerate_directdraw_6_devices.h>
#include <kelpo_renderer/surface/direct3d_6/surface_direct3d_6.h>
#include <kelpo_renderer/rasterizer/direct3d_6/rasterizer_direct3d_6.h>
#include <kelpo_renderer/window/win32/window_win32.h>
#include <kelpo_interface/error.h>

#include <windows.h>
#include <d3d.h>

LPDIRECTDRAW DIRECTDRAW_6 = NULL;
static LPDIRECTDRAWSURFACE FRONT_BUFFER = NULL;
static LPDIRECTDRAWSURFACE BACK_BUFFER = NULL;
static LPDIRECTDRAWSURFACE Z_BUFFER = NULL;

static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static unsigned WINDOW_BIT_DEPTH = 0;
static HWND WINDOW_HANDLE = 0;

int kelpo_surface_directdraw_6__lock_surface(LPDDSURFACEDESC surfaceDesc)
{
    HRESULT hr = 0;

    assert(surfaceDesc &&
           "Expected a non-null pointer to a surface descriptor struct.");

    surfaceDesc->dwSize = sizeof(surfaceDesc[0]);

    if (FAILED(hr = IDirectDrawSurface3_Lock(BACK_BUFFER, NULL, surfaceDesc, DDLOCK_WAIT, NULL)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        memset(surfaceDesc, 0, sizeof(surfaceDesc[0]));
        return 0;
    }

    return 1;
}

int kelpo_surface_directdraw_6__unlock_surface(void)
{
    HRESULT hr = 0;

    if (FAILED(hr = IDirectDrawSurface3_Unlock(BACK_BUFFER, NULL)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }
    
    return 1;
}

void kelpo_surface_directdraw_6__release_surface(void)
{
    if (BACK_BUFFER) IDirectDrawSurface3_DeleteAttachedSurface(BACK_BUFFER, 0, NULL);
    if (BACK_BUFFER) IDirectDrawSurface3_Release(BACK_BUFFER);
    if (Z_BUFFER) IDirectDrawSurface3_Release(Z_BUFFER);
    if (FRONT_BUFFER) IDirectDrawSurface3_Release(FRONT_BUFFER);
    if (DIRECTDRAW_6) IDirectDraw_Release(DIRECTDRAW_6);

    return;
}

void kelpo_surface_directdraw_6__flip_surface(const int vsyncEnabled)
{
    HRESULT hr = 0;

    /* TODO: Disabling vsync doesn't work. DirectDraw 7 has the DDFLIP_NOVSYNC
     * flag (c.f. DDFLIP_WAIT), but it doesn't seem to exist in DirectDraw 5.*/
    if (FAILED(hr = IDirectDrawSurface3_Flip(FRONT_BUFFER, NULL, (vsyncEnabled? DDFLIP_WAIT : 0))))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
    }

    return;
}

HRESULT kelpo_surface_directdraw_6__initialize_direct3d_6_interface(LPDIRECT3D2 *d3d,
                                                                    LPDIRECT3DDEVICE2 *d3dDevice)
{
    HRESULT hr = 0;

    assert((DIRECTDRAW_6 &&
            BACK_BUFFER) &&
           "Attempting to create a Direct3D interface without first properly initializing DirectDraw.");

    if (FAILED(hr = IDirectDraw_QueryInterface(DIRECTDRAW_6, IID_IDirect3D2, (void**)d3d)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return hr;
    }

    if (FAILED(hr = IDirect3D2_CreateDevice(*d3d, IID_IDirect3DHALDevice, BACK_BUFFER, d3dDevice)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return hr;
    }

    return S_OK;
}

HRESULT kelpo_surface_directdraw_6__initialize_direct3d_6_zbuffer(LPDIRECT3DDEVICE2 d3dDevice,
                                                                  const unsigned bitDepth)
{
    HRESULT hr = 0;
    DDSURFACEDESC zBufferSurfaceDesc;

    assert(BACK_BUFFER &&
           "Attempting to create a Z buffer before a back buffer has been created.");

    assert(DIRECTDRAW_6 &&
           "Attempting to create a Z buffer before a DirectDraw interface has been created.");

    zBufferSurfaceDesc.dwSize = sizeof(zBufferSurfaceDesc);
    
    if (FAILED(hr = IDirectDrawSurface3_GetSurfaceDesc(BACK_BUFFER, &zBufferSurfaceDesc)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return hr;
    }

    zBufferSurfaceDesc.dwFlags = (DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_ZBUFFERBITDEPTH);
    zBufferSurfaceDesc.ddsCaps.dwCaps = (DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY);
    zBufferSurfaceDesc.dwZBufferBitDepth = bitDepth;

    if (FAILED(hr = IDirectDraw_CreateSurface(DIRECTDRAW_6, &zBufferSurfaceDesc, &Z_BUFFER, NULL)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return hr;
    }

    if (FAILED(hr = IDirectDrawSurface3_AddAttachedSurface(BACK_BUFFER, Z_BUFFER)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return hr;
    }

    if (FAILED(hr = IDirect3DDevice2_SetRenderTarget(d3dDevice, BACK_BUFFER, 0)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return hr;
    }
    
    return S_OK;
}

HRESULT kelpo_surface_directdraw_6__initialize_surface(const unsigned width,
                                                       const unsigned height,
                                                       const unsigned bpp,
                                                       const HWND windowHandle,
                                                       GUID directDrawDeviceGUID)
{
    HRESULT hr = 0;
    WINDOW_HANDLE = windowHandle;
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    WINDOW_BIT_DEPTH = bpp;

    assert((WINDOW_HANDLE &&
            WINDOW_WIDTH &&
            WINDOW_HEIGHT) &&
           "DirectDraw 6: Attempting to initialize DirectDraw without a valid window handle.");

    assert((!DIRECTDRAW_6 &&
            !FRONT_BUFFER &&
            !BACK_BUFFER &&
            !Z_BUFFER) &&
           "DirectDraw 6: Attempting to doubly initialize DirectDraw.");

    /* Initialize DirectDraw with support for Direct3D.*/
    {
        const DWORD cooperativeLevel = (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);

        if (FAILED(hr = DirectDrawCreate(&directDrawDeviceGUID, &DIRECTDRAW_6, NULL)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_API_CALL_FAILED);
            return hr;
        }

        if (FAILED(hr = IDirectDraw_SetCooperativeLevel(DIRECTDRAW_6, WINDOW_HANDLE, cooperativeLevel)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_DISPLAY_MODE_NOT_SUPPORTED);
            return hr;
        }

        if (FAILED(hr = IDirectDraw_SetDisplayMode(DIRECTDRAW_6, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BIT_DEPTH)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_DISPLAY_MODE_NOT_SUPPORTED);
            return hr;
        }
    }

    /* Create a front buffer with an attached back buffer.*/
    {
        DDSURFACEDESC frontBufferSurfaceDesc;
        DDSCAPS backBufferCaps;
        
        memset(&backBufferCaps, 0, sizeof(DDSCAPS));
        backBufferCaps.dwCaps = DDSCAPS_BACKBUFFER;

        memset(&frontBufferSurfaceDesc, 0, sizeof(DDSURFACEDESC));
        frontBufferSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
        frontBufferSurfaceDesc.dwFlags = (DDSD_CAPS | DDSD_BACKBUFFERCOUNT);
        frontBufferSurfaceDesc.ddsCaps.dwCaps = (DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE | DDSCAPS_FLIP | DDSCAPS_COMPLEX);
        frontBufferSurfaceDesc.dwBackBufferCount = 1;

        if (FAILED(hr = IDirectDraw_CreateSurface(DIRECTDRAW_6, &frontBufferSurfaceDesc, &FRONT_BUFFER, NULL)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_API_CALL_FAILED);
            return hr;
        }

        if (FAILED(hr = IDirectDrawSurface3_GetAttachedSurface(FRONT_BUFFER, &backBufferCaps, &BACK_BUFFER)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_API_CALL_FAILED);
            return hr;
        }
    }

    return S_OK;
}
