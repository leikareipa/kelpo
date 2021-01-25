/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x render surface for the Kelpo renderer.
 * 
 */

#include <assert.h>
#include <kelpo_renderer/surface/glide_3/surface_glide_3.h>
#include <kelpo_renderer/rasterizer/glide_3/rasterizer_glide_3.h>
#include <kelpo_renderer/window/win32/window_win32.h>
#include <kelpo_interface/error.h>

#include <windows.h>
#include <glide/glide.h>

static GrContext_t GLIDE_RENDER_CONTEXT = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static const unsigned WINDOW_BIT_DEPTH = 16;
static unsigned IS_VSYNC_ENABLED = 0; /* Either 1 or 0, to set whether to use vsync.*/
static HWND WINDOW_HANDLE = 0;

void kelpo_surface_glide_3__release_surface(void)
{
    assert(GLIDE_RENDER_CONTEXT && "Can't release a NULL render context.");

    grSstWinClose(GLIDE_RENDER_CONTEXT);

    kelpo_window__release_window();

    return;
}

void kelpo_surface_glide_3__flip_surface(void)
{
    grBufferSwap(IS_VSYNC_ENABLED);

    return;
}

static LRESULT window_proc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        default: break;
    }

    return 0;
}

void kelpo_surface_glide_3__create_surface(const unsigned width,
                                           const unsigned height,
                                           const unsigned bpp,
                                           const int vsyncEnabled,
                                           const unsigned deviceIdx)
{
    GrScreenResolution_t glideResolution = GR_RESOLUTION_NONE;
    GrScreenRefresh_t glideRefreshRate = GR_REFRESH_60Hz;

    assert((bpp == 16) && "The render window must have 16-bit color.");

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    /* The window bit depth is ignored; must always be 16.*/
    IS_VSYNC_ENABLED = (vsyncEnabled? 1 : 0);

    assert(!GLIDE_RENDER_CONTEXT && "A render context already exists.");

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
    else assert(0 && "Unsupported resolution.");

    kelpo_window__open_window(WINDOW_WIDTH, WINDOW_HEIGHT, "Glide 3", window_proc);
    WINDOW_HANDLE = (HWND)kelpo_window__get_window_handle();

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);

    grGlideInit();
    grSstSelect(deviceIdx);
    GLIDE_RENDER_CONTEXT = grSstWinOpen((FxU32)WINDOW_HANDLE,
                                        glideResolution,
                                        glideRefreshRate,
                                        GR_COLORFORMAT_ARGB,
                                        GR_ORIGIN_UPPER_LEFT,
                                        2,
                                        1);

    if (!GLIDE_RENDER_CONTEXT)
    {
        /* TODO: Return false.*/
        kelpo_error(KELPOERR_DISPLAY_MODE_NOT_SUPPORTED);
    }

    return;
}
