/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A Direct3D 7 rasterizer surface for the Kelpo renderer.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <kelpo_renderer/surface/directdraw_7/enumerate_directdraw_7_devices.h>
#include <kelpo_renderer/surface/directdraw_7/surface_directdraw_7.h>
#include <kelpo_renderer/surface/direct3d_7/surface_direct3d_7.h>
#include <kelpo_renderer/window/win32/window_win32.h>
#include <kelpo_interface/error.h>

#include <windows.h>
#include <d3d.h>

static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static unsigned WINDOW_BIT_DEPTH = 0;
static unsigned IS_VSYNC_ENABLED = 0; /* Either 1 or 0, to set whether to use vsync.*/
static HWND WINDOW_HANDLE = 0;

static LPDIRECT3D7 DIRECT3D_7 = NULL;
LPDIRECT3DDEVICE7 D3DDEVICE_7 = NULL; /* This will be accessed by the rasterizer also.*/

/* Used in conjunction with IDirect3D7_EnumZBufferFormats(); matches the first
 * pixel format that supports a Z buffer, and copies it into 'dest'.*/
static HRESULT WINAPI enumerate_zbuffer_pixel_formats(DDPIXELFORMAT *pddpf,
                                                      VOID *dest)
{
    if (pddpf->dwRGBBitCount == 16) /* TODO: Match the actual intended bit depth.*/
    {
        memcpy(dest, pddpf, sizeof(DDPIXELFORMAT));
        return D3DENUMRET_CANCEL;
    }

    return D3DENUMRET_OK;
}

/* Sets up a hardware Direct3D rasterizer along with a DirectDraw surface to
 * render into.*/
static int setup_direct3d(GUID deviceGUID)
{
    HRESULT hr = 0;
    D3DVIEWPORT7 vp = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 1};

    assert(WINDOW_HANDLE &&
           "Attempting to initialize without a valid window handle.");

    if (!kelpo_surface_directdraw_7__initialize_surface(WINDOW_WIDTH,
                                                        WINDOW_HEIGHT,
                                                        WINDOW_BIT_DEPTH,
                                                        WINDOW_HANDLE,
                                                        deviceGUID))
    {
        return 0;
    }

    if (!kelpo_surface_directdraw_7__initialize_direct3d_7_interface(&DIRECT3D_7, &D3DDEVICE_7))
    {
        return 0;
    }

    if (FAILED(hr = IDirect3DDevice7_SetViewport(D3DDEVICE_7, &vp)))
    {
        fprintf(stderr, "Direct3D error 0x%x\n", hr);
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    /* Request a Z buffer.*/
    {
        DDPIXELFORMAT zBufferPixelFormat;

        /* TODO: Test if the device supports z-bufferless hidden surface removal
         * (D3DPRASTERCAPS_ZBUFFERLESSHSR).*/
 
        /* Find a suitable pixel format for the Z buffer.*/
        IDirect3D7_EnumZBufferFormats(DIRECT3D_7,
#ifdef __cplusplus
                                      IID_IDirect3DHALDevice,
#else
                                      &IID_IDirect3DHALDevice,
#endif
                                      enumerate_zbuffer_pixel_formats, (VOID*)&zBufferPixelFormat);

        /* We should find the pixel format struct properly initialized if a suitable
         * pixel format was obtained.*/
        if (zBufferPixelFormat.dwSize != sizeof(zBufferPixelFormat))
        {
            kelpo_error(KELPOERR_Z_BUFFERING_NOT_SUPPORTED);
            return 0;
        }

        if (!kelpo_surface_directdraw_7__initialize_direct3d_7_zbuffer(D3DDEVICE_7, &zBufferPixelFormat))
        {
            return 0;
        }
    }

    return 1;
}

int kelpo_surface_direct3d_7__release_surface(void)
{
    if (D3DDEVICE_7) IDirect3DDevice7_Release(D3DDEVICE_7);
    kelpo_surface_directdraw_7__release_surface();
    if (DIRECT3D_7) IDirect3D7_Release(DIRECT3D_7);

    return 1;
}

int kelpo_surface_direct3d_7__flip_surface(void)
{
    return kelpo_surface_directdraw_7__flip_surface(IS_VSYNC_ENABLED);
}

LRESULT kelpo_surface_direct3d_7__window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        default: break;
    }

    return 0;
}

int kelpo_surface_direct3d_7__create_surface(const unsigned width,
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

    if (!setup_direct3d(kelpo_directdraw7_device_guid(deviceIdx)))
    {
        return 0;
    }

    return 1;
}
