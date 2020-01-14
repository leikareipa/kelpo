/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x render surface for the shiet renderer.
 * 
 */

#include <assert.h>
#include <shiet_renderer/surface/glide_3/surface_glide_3.h>
#include <shiet_renderer/window/window_win32.h>

#include <windows.h>
#include <glide/glide.h>

static GrContext_t GLIDE_RENDER_CONTEXT = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static const unsigned WINDOW_BIT_DEPTH = 16;
static HWND WINDOW_HANDLE = 0;

void shiet_surface_glide_3__release_surface(void)
{
    assert(GLIDE_RENDER_CONTEXT &&
           "Glide 3.x: Can't release a NULL render context.");

    grSstWinClose(GLIDE_RENDER_CONTEXT);

    return;
}

void shiet_surface_glide_3__flip_surface(void)
{
    grBufferSwap(1);

    return;
}

void shiet_surface_glide_3__create_surface(const unsigned width,
                                           const unsigned height,
                                           const unsigned bpp,
                                           const unsigned deviceIdx)
{
    GrScreenResolution_t glideResolution = GR_RESOLUTION_NONE;
    GrScreenRefresh_t glideRefreshRate = GR_REFRESH_60Hz;

    assert((bpp == 16) &&
           "Glide 3.x: The render window must have 16-bit color.");

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    /* The window bit depth is ignored; must always be 16.*/

    assert(!GLIDE_RENDER_CONTEXT &&
           "Glide 3.x: A render context already exists.");

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
    else assert(0 && "Glide 3.x: Unsupported resolution.");

    shiet_window__create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "", NULL);
    WINDOW_HANDLE = (HWND)shiet_window__get_window_handle();

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
    
    assert(GLIDE_RENDER_CONTEXT &&
           "Glide 3.x: Failed to initialize the renderer.");

    return;
}
