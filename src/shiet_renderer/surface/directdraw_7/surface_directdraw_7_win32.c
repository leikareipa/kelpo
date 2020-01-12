/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Provides a Direct3D-compatible DirectDraw 7 surface for the shiet renderer.
 * 
 * Not intended for use as a standalone surface, but as an intermediary for
 * other surfaces that require a DirectDraw 7 backend.
 * 
 * Note: The Direct3D 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of shiet.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <shiet_renderer/rasterizer/direct3d_7/enumerate_directdraw_7_devices.h>
#include <shiet_renderer/surface/direct3d_7/surface_direct3d_7_win32.h>
#include <shiet_renderer/window/win32/window_win32.h>

#include <windows.h>
#include <d3d.h>

static LPDIRECTDRAW7 DIRECTDRAW_7 = NULL;
static LPDIRECTDRAWSURFACE7 FRONT_BUFFER = NULL;
static LPDIRECTDRAWSURFACE7 BACK_BUFFER = NULL;
static LPDIRECTDRAWSURFACE7 Z_BUFFER = NULL;

const static unsigned WINDOW_BIT_DEPTH = 16;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static HWND WINDOW_HANDLE = 0;

void shiet_surface_directdraw_7_win32__release_surface(void)
{
    if (FRONT_BUFFER) IDirectDrawSurface7_Release(FRONT_BUFFER);
    if (BACK_BUFFER) IDirectDrawSurface7_Release(BACK_BUFFER);
    if (Z_BUFFER) IDirectDrawSurface7_Release(Z_BUFFER);
    if (DIRECTDRAW_7) IDirectDraw7_Release(DIRECTDRAW_7);

    return;
}

void shiet_surface_directdraw_7_win32__flip_surface(void)
{
    IDirectDrawSurface7_Flip(FRONT_BUFFER, NULL, DDFLIP_WAIT);

    return;
}

HRESULT shiet_surface_directdraw_7_win32__initialize_direct3d_7_interface(LPDIRECT3D7 *d3d,
                                                                          LPDIRECT3DDEVICE7 *d3dDevice)
{
    HRESULT hr = 0;

    assert((DIRECTDRAW_7 &&
            BACK_BUFFER) &&
           "DirectDraw 7: Attempting to create a Direct3D interface without first properly initializing DirectDraw.");

#ifdef __cplusplus
    if (FAILED(hr = IDirectDraw7_QueryInterface(DIRECTDRAW_7, IID_IDirect3D7, (void**)d3d)))
#else
    if (FAILED(hr = IDirectDraw7_QueryInterface(DIRECTDRAW_7, &IID_IDirect3D7, d3d)))
#endif
    {
        fprintf(stderr, "DirectDraw 7: Failed to establish a Direct3D interface (error 0x%x).\n", hr);
        return hr;
    }

#ifdef __cplusplus
    if (FAILED(hr = IDirect3D7_CreateDevice(*d3d, IID_IDirect3DHALDevice, BACK_BUFFER, d3dDevice)))
#else
    if (FAILED(hr = IDirect3D7_CreateDevice(*d3d, &IID_IDirect3DHALDevice, BACK_BUFFER, d3dDevice)))
#endif
    {
        fprintf(stderr, "DirectDraw 7: Failed to create a Direct3D device (error 0x%x).\n", hr);
        return hr;
    }

    return S_OK;
}

HRESULT shiet_surface_directdraw_7_win32__initialize_direct3d_7_zbuffer(LPDIRECT3DDEVICE7 d3dDevice,
                                                                        LPDDPIXELFORMAT pixelFormat)
{
    HRESULT hr = 0;
    DDSURFACEDESC2 zBufferSurfaceDesc;

    assert(BACK_BUFFER &&
           "DirectDraw 7: Attempting to create a Z buffer before a back buffer has been created.");

    assert(DIRECTDRAW_7 &&
           "DirectDraw 7: Attempting to create a Z buffer before a DirectDraw interface has been created.");

    zBufferSurfaceDesc.dwSize = sizeof(zBufferSurfaceDesc);
    if (FAILED(hr = IDirectDrawSurface7_GetSurfaceDesc(BACK_BUFFER, &zBufferSurfaceDesc)))
    {
        fprintf(stderr, "DirectDraw 7: Failed to access the back buffer's description (error 0x%x).\n", hr);
        return hr;
    }

    zBufferSurfaceDesc.dwFlags = (DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT);
    zBufferSurfaceDesc.ddsCaps.dwCaps = (DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY);
    memcpy(&zBufferSurfaceDesc.ddpfPixelFormat, pixelFormat, sizeof(pixelFormat[0]));
    if (FAILED(hr = IDirectDraw7_CreateSurface(DIRECTDRAW_7, &zBufferSurfaceDesc, &Z_BUFFER, NULL)))
    {
        fprintf(stderr, "DirectDraw 7: Failed to create the Z buffer (error 0x%x).\n", hr);
        return hr;
    }

    if (FAILED(hr = IDirectDrawSurface7_AddAttachedSurface(BACK_BUFFER, Z_BUFFER)))
    {
        fprintf(stderr, "DirectDraw 7: Failed to attach the Z buffer (error 0x%x).\n", hr);
        return hr;
    }

    if (FAILED(hr = IDirect3DDevice7_SetRenderTarget(d3dDevice, BACK_BUFFER, 0)))
    {
        fprintf(stderr, "DirectDraw 7: A call to IDirect3DDevice7_SetRenderTarget() failed (error 0x%x).\n", hr);
        return hr;
    }
    
    return S_OK;
}

HRESULT shiet_surface_directdraw_7_win32__initialize_surface(const unsigned width,
                                                             const unsigned height,
                                                             const HWND windowHandle,
                                                             GUID deviceGUID)
{
    HRESULT hr = 0;
    WINDOW_HANDLE = windowHandle;
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    assert((WINDOW_HANDLE &&
            WINDOW_WIDTH &&
            WINDOW_HEIGHT) &&
           "DirectDraw 7: Attempting to initialize DirectDraw without a valid window handle.");

    assert((!DIRECTDRAW_7 &&
            !FRONT_BUFFER &&
            !BACK_BUFFER &&
            !Z_BUFFER) &&
           "DirectDraw 7: Attempting to doubly initialize DirectDraw.");

    /* Initialize DirectDraw with support for Direct3D.*/
    {
        const DWORD cooperativeLevel = (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);

#ifdef __cplusplus
        if (FAILED(hr = DirectDrawCreateEx(&deviceGUID, (VOID**)&DIRECTDRAW_7, IID_IDirectDraw7, NULL)))
#else
        if (FAILED(hr = DirectDrawCreateEx(&deviceGUID, (VOID**)&DIRECTDRAW_7, &IID_IDirectDraw7, NULL)))
#endif
        {
            fprintf(stderr, "DirectDraw 7: A call to DirectDrawCreateEx() failed (error 0x%x).\n", hr);
            return hr;
        }

        if (FAILED(hr = IDirectDraw7_SetCooperativeLevel(DIRECTDRAW_7, WINDOW_HANDLE, cooperativeLevel)))
        {
            fprintf(stderr, "DirectDraw 7: A call to IDirectDraw7_SetCooperativeLevel() failed (error 0x%x).\n", hr);
            return hr;
        }

        if (FAILED(hr = IDirectDraw7_SetDisplayMode(DIRECTDRAW_7, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BIT_DEPTH, 0, 0)))
        {
            fprintf(stderr, "DirectDraw 7: A call to IDirectDraw7_SetDisplayMode() failed (error 0x%x).\n", hr);
            return hr;
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
            fprintf(stderr, "DirectDraw 7: Failed to create a front buffer (error 0x%x).\n", hr);
            return hr;
        }

        if (FAILED(hr = IDirectDrawSurface7_GetAttachedSurface(FRONT_BUFFER, &backBufferCaps, &BACK_BUFFER)))
        {
            fprintf(stderr, "DirectDraw 7: Failed to create a back buffer (error 0x%x).\n", hr);
            return hr;
        }
    }

	return S_OK;
}
