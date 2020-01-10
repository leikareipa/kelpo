/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Direct3D 7 rasterizer for the shiet renderer.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <shiet_renderer/rasterizer/direct3d_7/surface_direct3d_7_win32.h>
#include <shiet_renderer/window/win32/window_win32.h>

#include <windows.h>
#include <d3d.h>

static RECT                 SCREEN_RECT;
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
    if (pddpf->dwFlags == DDPF_ZBUFFER)
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
static HRESULT initialize_direct3d(void)
{
	HRESULT hr;
    DDSURFACEDESC2 ddsd;

    assert(WINDOW_HANDLE &&
           "Direct3D 7 renderer: Attempting to initialize without a valid window handle.");

    /* Initialize DirectDraw.*/
    {
        /* Create the IDirectDraw interface. The first parameter is the GUID,
         * which is allowed to be NULL. If there are more than one DirectDraw
         * drivers on the system, a NULL guid requests the primary driver. For 
         * non-GDI hardware cards like the 3DFX and PowerVR, the guid would need
         * to be explicity specified . (Note: these guids are normally obtained
         * from enumeration, which is convered in a subsequent tutorial.)*/
#ifdef __cplusplus
        if (FAILED(hr = DirectDrawCreateEx(NULL, (VOID**)&DIRECTDRAW_7, IID_IDirectDraw7, NULL)))
#else
        if (FAILED(hr = DirectDrawCreateEx(NULL, (VOID**)&DIRECTDRAW_7, &IID_IDirectDraw7, NULL)))
#endif
        {
            return hr;
        }

        /* Set the Windows cooperative level. This is where we tell the system
         * whether wew will be rendering in fullscreen mode or in a window. Note
         * that some hardware (non-GDI) may not be able to render into a window.
         * The flag DDSCL_NORMAL specifies windowed mode. Using fullscreen mode
         * is the topic of a subsequent tutorial. The DDSCL_FPUSETUP flag is a 
         * hint to DirectX to optomize floating points calculations. See the docs
         * for more info on this. Note: this call could fail if another application
         * already controls a fullscreen, exclusive mode.*/
        if (FAILED(hr = IDirectDraw7_SetCooperativeLevel(DIRECTDRAW_7, WINDOW_HANDLE, DDSCL_NORMAL)))
        {
            return hr;
        }
    }

    /* Create the DirectDraw surfaces.*/
    {
        LPDIRECTDRAWCLIPPER pcClipper;

        /* Initialize a surface description structure for the primary surface. The
         * primary surface represents the entire display, with dimensions and a
         * pixel format of the display. Therefore, none of that information needs
         * to be specified in order to create the primary surface.*/
        ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        /* Create the primary surface.*/
        if (FAILED(hr = IDirectDraw7_CreateSurface(DIRECTDRAW_7, &ddsd, &SURFACE_FRONT, NULL)))
        {
            return hr;
        }

        /* Setup a surface description to create a backbuffer. This is an
         * offscreen plain surface with dimensions equal to our window size.
         * The DDSCAPS_3DDEVICE is needed so we can later query this surface
         * for an IDirect3DDevice interface.*/
        ddsd.dwFlags = (DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS);
        ddsd.ddsCaps.dwCaps = (DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE);

        /* Set the dimensions of the backbuffer. Note that if our window changes
         * size, we need to destroy this surface and create a new one.*/
        GetClientRect(WINDOW_HANDLE, &SCREEN_RECT);
        ClientToScreen(WINDOW_HANDLE, (POINT*)&SCREEN_RECT.left);
        ClientToScreen(WINDOW_HANDLE, (POINT*)&SCREEN_RECT.right);
        ddsd.dwWidth  = WINDOW_WIDTH;
        ddsd.dwHeight = WINDOW_HEIGHT;

        /* Create the backbuffer. The most likely reason for failure is running
         * out of video memory. (A more sophisticated app should handle this.)*/
        if (FAILED(hr = IDirectDraw7_CreateSurface(DIRECTDRAW_7, &ddsd, &SURFACE_BACK, NULL)))
        {
            return hr;
        }

        /* Create a clipper object which handles all our clipping for cases when
         * our window is partially obscured by other windows. This is not needed
         * for apps running in fullscreen mode.*/
        if (FAILED(hr = IDirectDraw7_CreateClipper(DIRECTDRAW_7, 0, &pcClipper, NULL)))
        {
            return hr;
        }

        /* Associate the clipper with our window. Note that, afterwards, the
         * clipper is internally referenced by the primary surface, so it is safe
         * to release our local reference to it.*/
        IDirectDrawClipper_SetHWnd(pcClipper, 0, WINDOW_HANDLE);
        IDirectDrawSurface7_SetClipper(SURFACE_FRONT, pcClipper);
        IDirectDrawClipper_Release(pcClipper);
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
            return hr;
        }

        /* Create a Z buffer.*/
        {
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

            ddsd.dwFlags = (DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT);
            ddsd.ddsCaps.dwCaps = (DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY);
            ddsd.dwWidth = WINDOW_WIDTH;
            ddsd.dwHeight = WINDOW_HEIGHT;
            memcpy(&ddsd.ddpfPixelFormat, &ddpfZBuffer, sizeof(DDPIXELFORMAT));

            /* Create and attach a z-buffer. Real apps should be able to handle an
             * error here (DDERR_OUTOFVIDEOMEMORY may be encountered). For this 
             * tutorial, though, we are simply going to exit ungracefully.*/
            if (FAILED(hr = IDirectDraw7_CreateSurface(DIRECTDRAW_7, &ddsd, &Z_BUFFER, NULL)))
            {
                return hr;
            }

            /* Attach the z-buffer to the back buffer.*/
            if (FAILED(hr = IDirectDrawSurface7_AddAttachedSurface(SURFACE_BACK, Z_BUFFER)))
            {
                return hr;
            }
        }

        /* Before creating the device, check that we are NOT in a palettized
         * display. That case will cause CreateDevice() to fail, since this simple 
         * tutorial does not bother with palettes.*/
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        IDirectDraw7_GetDisplayMode(DIRECTDRAW_7, &ddsd);
        if (ddsd.ddpfPixelFormat.dwRGBBitCount <= 8)
        {
            return DDERR_INVALIDMODE;
        }

        /* Create the device. The GUID is hardcoded for now, but should come from
         * device enumeration, which is the topic of a future tutorial. The device
         * is created off of our back buffer, which becomes the render target for
         * the newly created device.*/
#ifdef __cplusplus
        if (FAILED(hr = IDirect3D7_CreateDevice(DIRECT3D_7, IID_IDirect3DHALDevice, SURFACE_BACK, &D3DDEVICE_7)))
#else
        if (FAILED(hr = IDirect3D7_CreateDevice(DIRECT3D_7, &IID_IDirect3DHALDevice, SURFACE_BACK, &D3DDEVICE_7)))
#endif
        {
            return hr;
            
            /* This call could fail for many reasons. The most likely cause is
             * that we specifically requested a hardware device, without knowing
             * whether there is even a 3D card installed in the system. Another
             * possibility is the hardware is incompatible with the current display
             * mode (once again, the correct implementation would use enumeration
             * to check that). In any case, let's simply try again with the RGB
             * software rasterizer.*/
#ifdef __cplusplus
            if (FAILED(hr = IDirect3D7_CreateDevice(DIRECT3D_7, IID_IDirect3DRGBDevice, SURFACE_BACK, &D3DDEVICE_7)))
#else
            if (FAILED(hr = IDirect3D7_CreateDevice(DIRECT3D_7, &IID_IDirect3DRGBDevice, SURFACE_BACK, &D3DDEVICE_7)))
#endif
            {
                return hr;
            }
        }
    }

    /* Create the viewport.*/
    {
        D3DVIEWPORT7 vp = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 1};

        if (FAILED(hr = IDirect3DDevice7_SetViewport(D3DDEVICE_7, &vp)))
        {
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

void shiet_surface_direct3d_7_win32__update_surface(void)
{
    MSG m;

    while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }

    IDirectDraw7_WaitForVerticalBlank(DIRECTDRAW_7, DDWAITVB_BLOCKBEGIN, NULL);
    IDirectDrawSurface7_Blt(SURFACE_FRONT, &SCREEN_RECT, SURFACE_BACK, NULL, DDBLT_WAIT, NULL);

    return;
}

static LRESULT window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        /* Update blit region with the window's new position.*/
        case WM_MOVE:
        {
            const unsigned x = LOWORD(lParam);
            const unsigned y = HIWORD(lParam);

            SetRect(&SCREEN_RECT, x, y, (x + WINDOW_WIDTH), (y + WINDOW_HEIGHT));

            break;
        }

        default: break;
    }

    return 0;
}

void shiet_surface_direct3d_7_win32__create_surface(const unsigned width,
                                                    const unsigned height,
                                                    const char *const windowTitle)
{
    shiet_window_win32__create_window(width, height, windowTitle, window_proc);
    WINDOW_HANDLE = (HWND)shiet_window_win32__get_window_handle();

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);
    UpdateWindow(WINDOW_HANDLE);

	if (FAILED(initialize_direct3d()))
	{
		assert(0 && "Direct3D 7 renderer: Failed to initialize the renderer.");
	}

    return;
}
