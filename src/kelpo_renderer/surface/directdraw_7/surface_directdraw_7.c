/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Provides a Direct3D-compatible DirectDraw 7 surface for the Kelpo renderer.
 * 
 * Not intended for use as a standalone surface, but as an intermediary for
 * other surfaces that require a DirectDraw 7 backend.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <kelpo_renderer/surface/directdraw_7/enumerate_directdraw_7_devices.h>
#include <kelpo_renderer/surface/direct3d_7/surface_direct3d_7.h>
#include <kelpo_renderer/rasterizer/direct3d_7/rasterizer_direct3d_7.h>
#include <kelpo_renderer/window/win32/window_win32.h>
#include <kelpo_interface/error.h>

#include <windows.h>
#include <d3d.h>

static LPDIRECTDRAW7 DIRECTDRAW_7 = NULL;
static LPDIRECTDRAWSURFACE7 FRONT_BUFFER = NULL;
static LPDIRECTDRAWSURFACE7 BACK_BUFFER = NULL;
static LPDIRECTDRAWSURFACE7 Z_BUFFER = NULL;

static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static unsigned WINDOW_BIT_DEPTH = 0;
static HWND WINDOW_HANDLE = 0;

int kelpo_surface_directdraw_7__lock_surface(LPDDSURFACEDESC2 surfaceDesc)
{
    HRESULT hr = 0;

    assert(surfaceDesc &&
           "Expected a non-null pointer to a surface descriptor struct.");

    surfaceDesc->dwSize = sizeof(surfaceDesc[0]);

    if (FAILED(hr = IDirectDrawSurface7_Lock(BACK_BUFFER, NULL, surfaceDesc, DDLOCK_WAIT, NULL)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        memset(surfaceDesc, 0, sizeof(surfaceDesc[0]));
        return 0;
    }

    return 1;
}

int kelpo_surface_directdraw_7__unlock_surface(void)
{
    HRESULT hr = 0;
    
    if (FAILED(hr = IDirectDrawSurface7_Unlock(BACK_BUFFER, NULL)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }
    
    return 1;
}

int kelpo_surface_directdraw_7__release_surface(void)
{
    if (BACK_BUFFER) IDirectDrawSurface7_DeleteAttachedSurface(BACK_BUFFER, 0, NULL);
    if (BACK_BUFFER) IDirectDrawSurface7_Release(BACK_BUFFER);
    if (Z_BUFFER) IDirectDrawSurface7_Release(Z_BUFFER);
    if (FRONT_BUFFER) IDirectDrawSurface7_Release(FRONT_BUFFER);
    if (DIRECTDRAW_7) IDirectDraw7_Release(DIRECTDRAW_7);

    return 1;
}

int kelpo_surface_directdraw_7__flip_surface(const int vsyncEnabled)
{
    HRESULT hr = 0;

    if (FAILED(hr = IDirectDrawSurface7_Flip(FRONT_BUFFER, NULL, (vsyncEnabled? DDFLIP_WAIT : DDFLIP_NOVSYNC))))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    return 1;
}

int kelpo_surface_directdraw_7__initialize_direct3d_7_interface(LPDIRECT3D7 *d3d,
                                                                LPDIRECT3DDEVICE7 *d3dDevice)
{
    HRESULT hr = 0;

    assert((DIRECTDRAW_7 &&
            BACK_BUFFER) &&
           "Attempting to create a Direct3D interface without first properly initializing DirectDraw.");

#ifdef __cplusplus
    if (FAILED(hr = IDirectDraw7_QueryInterface(DIRECTDRAW_7, IID_IDirect3D7, (void**)d3d)))
#else
    if (FAILED(hr = IDirectDraw7_QueryInterface(DIRECTDRAW_7, &IID_IDirect3D7, d3d)))
#endif
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

#ifdef __cplusplus
    if (FAILED(hr = IDirect3D7_CreateDevice(*d3d, IID_IDirect3DHALDevice, BACK_BUFFER, d3dDevice)))
#else
    if (FAILED(hr = IDirect3D7_CreateDevice(*d3d, &IID_IDirect3DHALDevice, BACK_BUFFER, d3dDevice)))
#endif
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    return 1;
}

int kelpo_surface_directdraw_7__initialize_direct3d_7_zbuffer(LPDIRECT3DDEVICE7 d3dDevice,
                                                              LPDDPIXELFORMAT pixelFormat)
{
    HRESULT hr = 0;
    DDSURFACEDESC2 zBufferSurfaceDesc;

    assert(BACK_BUFFER &&
           "Attempting to create a Z buffer before a back buffer has been created.");

    assert(DIRECTDRAW_7 &&
           "Attempting to create a Z buffer before a DirectDraw interface has been created.");

    zBufferSurfaceDesc.dwSize = sizeof(zBufferSurfaceDesc);

    if (FAILED(hr = IDirectDrawSurface7_GetSurfaceDesc(BACK_BUFFER, &zBufferSurfaceDesc)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    zBufferSurfaceDesc.dwFlags = (DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT);
    zBufferSurfaceDesc.ddsCaps.dwCaps = (DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY);
    memcpy(&zBufferSurfaceDesc.ddpfPixelFormat, pixelFormat, sizeof(pixelFormat[0]));
    
    if (FAILED(hr = IDirectDraw7_CreateSurface(DIRECTDRAW_7, &zBufferSurfaceDesc, &Z_BUFFER, NULL)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    if (FAILED(hr = IDirectDrawSurface7_AddAttachedSurface(BACK_BUFFER, Z_BUFFER)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    if (FAILED(hr = IDirect3DDevice7_SetRenderTarget(d3dDevice, BACK_BUFFER, 0)))
    {
        fprintf(stderr, "DirectDraw error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }
    
    return 1;
}

int kelpo_surface_directdraw_7__initialize_surface(const unsigned width,
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
           "Attempting to initialize DirectDraw without a valid window handle.");

    assert((!DIRECTDRAW_7 &&
            !FRONT_BUFFER &&
            !BACK_BUFFER &&
            !Z_BUFFER) &&
           "Attempting to doubly initialize DirectDraw.");

    /* Initialize DirectDraw with support for Direct3D.*/
    {
        const DWORD cooperativeLevel = (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);

#ifdef __cplusplus
        if (FAILED(hr = DirectDrawCreateEx(&directDrawDeviceGUID, (VOID**)&DIRECTDRAW_7, IID_IDirectDraw7, NULL)))
#else
        if (FAILED(hr = DirectDrawCreateEx(&directDrawDeviceGUID, (VOID**)&DIRECTDRAW_7, &IID_IDirectDraw7, NULL)))
#endif
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_API_CALL_FAILED);
            return 0;
        }

        if (FAILED(hr = IDirectDraw7_SetCooperativeLevel(DIRECTDRAW_7, WINDOW_HANDLE, cooperativeLevel)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_DISPLAY_MODE_NOT_SUPPORTED);
            return 0;
        }

        if (FAILED(hr = IDirectDraw7_SetDisplayMode(DIRECTDRAW_7, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BIT_DEPTH, 0, 0)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_DISPLAY_MODE_NOT_SUPPORTED);
            return 0;
        }
    }

    /* Create a front buffer with an attached back buffer.*/
    {
        DDSURFACEDESC2 frontBufferSurfaceDesc;
        DDSCAPS2 backBufferCaps;
        
        memset(&backBufferCaps, 0, sizeof(DDSCAPS2));
        backBufferCaps.dwCaps = DDSCAPS_BACKBUFFER;

        memset(&frontBufferSurfaceDesc, 0, sizeof(DDSURFACEDESC2));
        frontBufferSurfaceDesc.dwSize = sizeof(DDSURFACEDESC2);
        frontBufferSurfaceDesc.dwFlags = (DDSD_CAPS | DDSD_BACKBUFFERCOUNT);
        frontBufferSurfaceDesc.ddsCaps.dwCaps = (DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE | DDSCAPS_FLIP | DDSCAPS_COMPLEX);
        frontBufferSurfaceDesc.dwBackBufferCount = 1;

        if (FAILED(hr = IDirectDraw7_CreateSurface(DIRECTDRAW_7, &frontBufferSurfaceDesc, &FRONT_BUFFER, NULL)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_API_CALL_FAILED);
            return 0;
        }

        if (FAILED(hr = IDirectDrawSurface7_GetAttachedSurface(FRONT_BUFFER, &backBufferCaps, &BACK_BUFFER)))
        {
            fprintf(stderr, "DirectDraw error 0x%x\n", hr);
            kelpo_error(KELPOERR_API_CALL_FAILED);
            return 0;
        }
    }

    return 1;
}
