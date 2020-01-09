/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x render surface for the shiet renderer.
 * 
 */

#include <assert.h>
#include <shiet_renderer/rasterizer/glide_3/surface_glide_3_win32.h>
#include <shiet_renderer/window/win32/window_win32.h>

#include <windows.h>
#include <glide/glide.h>

static GrContext_t GLIDE_RENDER_CONTEXT = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static HWND WINDOW_HANDLE = 0;

void shiet_surface_glide_3_win32__release_surface(void)
{
    assert(GLIDE_RENDER_CONTEXT &&
           "Glide 3.x renderer: Can't release a NULL render context.");

    grSstWinClose(GLIDE_RENDER_CONTEXT);

    return;
}

void shiet_surface_glide_3_win32__update_surface(void)
{
    MSG m;

    InvalidateRect(WINDOW_HANDLE, NULL, FALSE);

    while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }

    grBufferSwap(1);

    return;
}

void shiet_surface_glide_3_win32__create_surface(const unsigned width,
                                                 const unsigned height,
                                                 const char *const windowTitle)
{
    GrScreenResolution_t glideResolution = GR_RESOLUTION_NONE;
    GrScreenRefresh_t glideRefreshRate = GR_REFRESH_60Hz;

    assert(!GLIDE_RENDER_CONTEXT &&
           "Glide 3.x renderer: A render context already exists.");

    if      ((width == 320)  && (height == 200))  glideResolution = GR_RESOLUTION_320x200;
    else if ((width == 320)  && (height == 240))  glideResolution = GR_RESOLUTION_320x240;
    else if ((width == 400)  && (height == 300))  glideResolution = GR_RESOLUTION_400x300;
    else if ((width == 512)  && (height == 384))  glideResolution = GR_RESOLUTION_512x384;
    else if ((width == 640)  && (height == 400))  glideResolution = GR_RESOLUTION_640x400;
    else if ((width == 640)  && (height == 480))  glideResolution = GR_RESOLUTION_640x480;
    else if ((width == 800)  && (height == 600))  glideResolution = GR_RESOLUTION_800x600;
    else if ((width == 1024) && (height == 768))  glideResolution = GR_RESOLUTION_1024x768;
    else if ((width == 1280) && (height == 1024)) glideResolution = GR_RESOLUTION_1280x1024;
    else if ((width == 1600) && (height == 1200)) glideResolution = GR_RESOLUTION_1600x1200;
    else assert(0 && "Glide 3.x renderer: Unsupported resolution.");

    shiet_window_win32__create_window(width, height, windowTitle, NULL);
    WINDOW_HANDLE = (HWND)shiet_window_win32__get_window_handle();

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);

    grGlideInit();
    grSstSelect(0);
    GLIDE_RENDER_CONTEXT = grSstWinOpen((FxU32)WINDOW_HANDLE,
                                        glideResolution,
                                        glideRefreshRate,
                                        GR_COLORFORMAT_ARGB,
                                        GR_ORIGIN_UPPER_LEFT,
                                        2,
                                        1);
    
    assert(GLIDE_RENDER_CONTEXT &&
           "Glide 3.x renderer: Failed to initialize the renderer.");

    return;
}
