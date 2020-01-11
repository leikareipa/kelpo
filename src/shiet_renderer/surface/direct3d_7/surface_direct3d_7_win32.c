/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Direct3D 7 rasterizer for the shiet renderer.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <shiet_renderer/surface/direct3d_7/surface_direct3d_7_win32.h>
#include <shiet_renderer/rasterizer/direct3d_7/enumerate_directdraw7_devices.h>
#include <shiet_renderer/window/win32/window_win32.h>

#include <windows.h>
#include <d3d.h>

static LPDIRECTDRAW7        DIRECTDRAW_7  = NULL;
static LPDIRECTDRAWSURFACE7 SURFACE_FRONT = NULL;
static LPDIRECTDRAWSURFACE7 SURFACE_BACK  = NULL;
static LPDIRECTDRAWSURFACE7 Z_BUFFER      = NULL;
static LPDIRECT3D7          DIRECT3D_7    = NULL;
LPDIRECT3DDEVICE7           D3DDEVICE_7   = NULL; /* This will be accessed by the rasterizer.*/

static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static HWND WINDOW_HANDLE = 0;

/* Used in conjunction with IDirect3D7_EnumZBufferFormats(); matches the first
 * pixel format that supports a Z buffer, and copies it into 'dest'.*/
static HRESULT WINAPI enumerate_zbuffer_pixel_formats(DDPIXELFORMAT* pddpf,
                                                      VOID* dest)
{
    if (pddpf->dwRGBBitCount == 16)
    {
        memcpy(dest, pddpf, sizeof(DDPIXELFORMAT));
		return D3DENUMRET_CANCEL;
    }

    return D3DENUMRET_OK;
}

/* Sets up a Direct3D renderer along with a DirectDraw surface to render into.
 *
 * Adapted from sample code provided by Microsoft with the DirectX 7 SDK. Comments
 * are largely kept as in the original.
 */
static HRESULT initialize_direct3d(GUID *const deviceGUID)
{
	HRESULT hr;
    DDSURFACEDESC2 ddsd;

    assert(WINDOW_HANDLE &&
           "Direct3D 7 renderer: Attempting to initialize without a valid window handle.");

    /* Initialize DirectDraw.*/
    {
#ifdef __cplusplus
        if (FAILED(hr = DirectDrawCreateEx(deviceGUID, (VOID**)&DIRECTDRAW_7, IID_IDirectDraw7, NULL)))
#else
        if (FAILED(hr = DirectDrawCreateEx(deviceGUID, (VOID**)&DIRECTDRAW_7, &IID_IDirectDraw7, NULL)))
#endif
        {
            fprintf(stderr, "A call to DirectDrawCreateEx() failed.\n");
            return hr;
        }

        if (FAILED(hr = IDirectDraw7_SetCooperativeLevel(DIRECTDRAW_7, WINDOW_HANDLE, (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT))))
        {
            fprintf(stderr, "A call to IDirectDraw7_SetCooperativeLevel() failed.\n");
            return hr;
        }

        if (FAILED(hr = IDirectDraw7_SetDisplayMode(DIRECTDRAW_7, WINDOW_WIDTH, WINDOW_HEIGHT, 16, 0, 0)))
        {
            fprintf(stderr, "A call to IDirectDraw7_SetDisplayMode() failed.\n");
            return hr;
        }
    }

    /* Create a front buffer with an attached backbuffer.*/
    {
        DDSCAPS2 ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0 };

        ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        ddsd.dwFlags = (DDSD_CAPS | DDSD_BACKBUFFERCOUNT);
        ddsd.ddsCaps.dwCaps = (DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE | DDSCAPS_FLIP | DDSCAPS_COMPLEX);
        ddsd.dwBackBufferCount = 1;

        if (FAILED(hr = IDirectDraw7_CreateSurface(DIRECTDRAW_7, &ddsd, &SURFACE_FRONT, NULL)))
        {
            fprintf(stderr, "Failed to create the front buffer.\n");
            return hr;
        }

        if (FAILED(hr = IDirectDrawSurface7_GetAttachedSurface(SURFACE_FRONT, &ddscaps, &SURFACE_BACK)))
        {
            fprintf(stderr, "Failed to create the back buffer.\n");
            return hr;
        }
    }

    /* Create Direct3D interfaces.*/
    {
        /* Query DirectDraw for access to Direct3D*/
#ifdef __cplusplus
        if (FAILED(hr = IDirectDraw7_QueryInterface(DIRECTDRAW_7, IID_IDirect3D7, (void**)&DIRECT3D_7)))
#else
        if (FAILED(hr = IDirectDraw7_QueryInterface(DIRECTDRAW_7, &IID_IDirect3D7, (void**)&DIRECT3D_7)))
#endif
        {
            fprintf(stderr, "A call to IDirectDraw7_QueryInterface() failed.\n");
            return hr;
        }

        /* Create the device. The device is created off of our back buffer, which
         * becomes the render target for the newly created device.*/
#ifdef __cplusplus
        if (FAILED(hr = IDirect3D7_CreateDevice(DIRECT3D_7, IID_IDirect3DHALDevice, SURFACE_BACK, &D3DDEVICE_7)))
#else
        if (FAILED(hr = IDirect3D7_CreateDevice(DIRECT3D_7, &IID_IDirect3DHALDevice, SURFACE_BACK, &D3DDEVICE_7)))
#endif
        {
            fprintf(stderr, "A call to IDirect3D7_CreateDevice() failed.\n");
            return hr;
        }
    }

    /* Create the viewport.*/
    {
        D3DVIEWPORT7 vp = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 1};

        if (FAILED(hr = IDirect3DDevice7_SetViewport(D3DDEVICE_7, &vp)))
        {
            fprintf(stderr, "A call to IDirect3DDevice7_SetViewport() failed.\n");
            return hr;
        }
    }

    /* Create a Z buffer.*/
    {
        /* TODO: Test if the device supports z-bufferless hidden surface removal
         *       (D3DPRASTERCAPS_ZBUFFERLESSHSR).*/
        
        DDPIXELFORMAT ddpfZBuffer;

        IDirect3D7_EnumZBufferFormats(DIRECT3D_7,
#ifdef __cplusplus
                                        IID_IDirect3DHALDevice,
#else
                                        &IID_IDirect3DHALDevice,
#endif
                                        enumerate_zbuffer_pixel_formats, (VOID*)&ddpfZBuffer);

        /* If we found a good zbuffer format, then the dwSize field will be
         * properly set during enumeration. Else, we have a problem and will exit.*/
        if (sizeof(DDPIXELFORMAT) != ddpfZBuffer.dwSize)
        {
            return E_FAIL;
        }

        IDirectDrawSurface7_GetSurfaceDesc(SURFACE_BACK, &ddsd);
        ddsd.dwFlags = (DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT);
        ddsd.ddsCaps.dwCaps = (DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY);
        memcpy(&ddsd.ddpfPixelFormat, &ddpfZBuffer, sizeof(DDPIXELFORMAT));

        /* Create and attach a z-buffer. Real apps should be able to handle an
         * error here (DDERR_OUTOFVIDEOMEMORY may be encountered). For this 
         * tutorial, though, we are simply going to exit ungracefully.*/
        if (FAILED(hr = IDirectDraw7_CreateSurface(DIRECTDRAW_7, &ddsd, &Z_BUFFER, NULL)))
        {
            fprintf(stderr, "A call to IDirectDraw7_CreateSurface() failed.\n");
            return hr;
        }

        /* Attach the z-buffer to the back buffer.*/
        if (FAILED(hr = IDirectDrawSurface7_AddAttachedSurface(SURFACE_BACK, Z_BUFFER)))
        {
            fprintf(stderr, "A call to IDirectDrawSurface7_AdddsdfAttachedSurface() failed.\n");
            return hr;
        }

        if (FAILED(hr = IDirect3DDevice7_SetRenderTarget(D3DDEVICE_7, SURFACE_BACK, 0)))
        {
            fprintf(stderr, "A call to IDirect3DDevice7_SetRenderTarget() failed.\n");
            return hr;
        }
    }

	return S_OK;
}

void shiet_surface_direct3d_7_win32__release_surface(void)
{
    if (SURFACE_FRONT) IDirectDrawSurface7_Release(SURFACE_FRONT);
    if (SURFACE_BACK)  IDirectDrawSurface7_Release(SURFACE_BACK);
    if (Z_BUFFER)      IDirectDrawSurface7_Release(Z_BUFFER);
    if (DIRECTDRAW_7)  IDirectDraw7_Release(DIRECTDRAW_7);
    if (DIRECT3D_7)    IDirect3D7_Release(DIRECT3D_7);
    if (D3DDEVICE_7)   IDirect3DDevice7_Release(D3DDEVICE_7);

    return;
}

void shiet_surface_direct3d_7_win32__flip_surface(void)
{
    IDirectDrawSurface7_Flip(SURFACE_FRONT, NULL, DDFLIP_WAIT);

    return;
}

static LRESULT window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        default: break;
    }

    return 0;
}

void shiet_surface_direct3d_7_win32__create_surface(const unsigned width,
                                                    const unsigned height)
{
    shiet_window_win32__create_window(width, height, "", window_proc);
    WINDOW_HANDLE = (HWND)shiet_window_win32__get_window_handle();

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);
    UpdateWindow(WINDOW_HANDLE);

	if (FAILED(initialize_direct3d(shiet_guid_of_directdraw7_device(0))))
	{
		assert(0 && "Direct3D 7 renderer: Failed to initialize the renderer.");
	}

    return;
}
