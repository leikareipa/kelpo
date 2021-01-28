/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A Direct3D 5 rasterizer surface for the Kelpo renderer.
 * 
 * Note: The DirectX 5 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <kelpo_renderer/surface/directdraw_5/enumerate_directdraw_5_devices.h>
#include <kelpo_renderer/surface/directdraw_5/surface_directdraw_5.h>
#include <kelpo_renderer/surface/direct3d_5/surface_direct3d_5.h>
#include <kelpo_renderer/window/win32/window_win32.h>
#include <kelpo_interface/error.h>

#include <windows.h>
#include <d3d.h>

static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static unsigned WINDOW_BIT_DEPTH = 0;
static unsigned IS_VSYNC_ENABLED = 0; /* Either 1 or 0, to set whether to use vsync.*/
static HWND WINDOW_HANDLE = 0;

static LPDIRECT3D2 DIRECT3D_5 = NULL;
LPDIRECT3DVIEWPORT2 D3DVIEWPORT_5 = NULL; /* This will be accessed by the rasterizer also.*/
LPDIRECT3DDEVICE2 D3DDEVICE_5 = NULL; /* This will be accessed by the rasterizer also.*/

/* Sets up a hardware Direct3D rasterizer along with a DirectDraw surface to
 * render into.*/
static int setup_direct3d(GUID deviceGUID)
{
    HRESULT hr = 0;
    D3DVIEWPORT viewport;

    assert(WINDOW_HANDLE &&
           "Attempting to initialize without a valid window handle.");

    memset(&viewport, 0, sizeof(viewport));
    viewport.dwSize = sizeof(viewport);
    viewport.dwWidth = WINDOW_WIDTH;
    viewport.dwHeight = WINDOW_HEIGHT;
    viewport.dvScaleX = WINDOW_WIDTH;
    viewport.dvScaleY = WINDOW_HEIGHT;

     if (!kelpo_surface_directdraw_5__initialize_surface(WINDOW_WIDTH,
                                                         WINDOW_HEIGHT,
                                                         WINDOW_BIT_DEPTH,
                                                         WINDOW_HANDLE,
                                                         deviceGUID))
    {
        return 0;
    }

    if (!kelpo_surface_directdraw_5__initialize_direct3d_5_interface(&DIRECT3D_5, &D3DDEVICE_5))
    {
        return 0;
    }

    if (FAILED(hr = IDirect3D2_CreateViewport(DIRECT3D_5, &D3DVIEWPORT_5, NULL)))
    {
        fprintf(stderr, "Direct3D error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    if (FAILED(hr = IDirect3DDevice2_AddViewport(D3DDEVICE_5, D3DVIEWPORT_5)))
    {
        fprintf(stderr, "Direct3D error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    if (FAILED(hr = IDirect3DViewport2_SetViewport(D3DVIEWPORT_5, &viewport)))
    {
        fprintf(stderr, "Direct3D error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    if (FAILED(hr = IDirect3DDevice2_SetCurrentViewport(D3DDEVICE_5, D3DVIEWPORT_5)))
    {
        fprintf(stderr, "Direct3D error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    if (!kelpo_surface_directdraw_5__initialize_direct3d_5_zbuffer(D3DDEVICE_5, 16))
    {
        return 0;
    }

    return 1;
}

int kelpo_surface_direct3d_5__release_surface(void)
{
    kelpo_surface_directdraw_5__release_surface();

    if (D3DDEVICE_5) IDirect3DDevice2_Release(D3DDEVICE_5);
    if (D3DVIEWPORT_5) IDirect3DViewport2_Release(D3DVIEWPORT_5);
    if (DIRECT3D_5) IDirect3D2_Release(DIRECT3D_5);

    return 1;
}

int kelpo_surface_direct3d_5__flip_surface(void)
{
    return kelpo_surface_directdraw_5__flip_surface(IS_VSYNC_ENABLED);;
}

LRESULT kelpo_surface_direct3d_5__window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        default: break;
    }

    return 0;
}

int kelpo_surface_direct3d_5__create_surface(const unsigned width,
                                             const unsigned height,
                                             const unsigned bpp,
                                             const int vsyncEnabled,
                                             const unsigned deviceIdx)
{
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    WINDOW_BIT_DEPTH = bpp;
    IS_VSYNC_ENABLED = (vsyncEnabled? 1 : 0);

    if (!(WINDOW_HANDLE = (HWND)kelpo_window__get_window_handle()))
    {
        return 0;
    }

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);
    UpdateWindow(WINDOW_HANDLE);

    if (!setup_direct3d(kelpo_directdraw5_device_guid(deviceIdx)))
    {
        return 0;
    }

    return 1;
}
