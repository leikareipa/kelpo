/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x render surface for the shiet renderer.
 * 
 */

#include <assert.h>
#include <shiet_lib/renderer/rasterizer/opengl/surface_opengl_win32.h>
#include <shiet_lib/renderer/window/win32/window_win32.h>

#include <windows.h>
#include <glide/glide.h>

static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static HDC WINDOW_DC = 0;
static HWND WINDOW_HANDLE = 0;
static GrContext_t GLIDE_RENDER_CONTEXT = 0;

void shiet_surface_glide3_win32__release_surface(void)
{
    assert(GLIDE_RENDER_CONTEXT &&
           "Glide 3.x renderer: Can't release a NULL render context.");

    grSstWinClose(GLIDE_RENDER_CONTEXT);

    return;
}

void shiet_surface_glide3_win32__update_surface(void)
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

void shiet_surface_glide3_win32__create_surface(const unsigned width,
                                                const unsigned height,
                                                const char *const windowTitle)
{
    int glideResolution = 0;

    assert(!GLIDE_RENDER_CONTEXT &&
           "Glide 3.x renderer: A render context already exists.");

    if      ((width == 320)  && (height == 200)) glideResolution = GR_RESOLUTION_320x200;
    else if ((width == 320)  && (height == 240)) glideResolution = GR_RESOLUTION_320x240;
    else if ((width == 400)  && (height == 300)) glideResolution = GR_RESOLUTION_400x300;
    else if ((width == 512)  && (height == 384)) glideResolution = GR_RESOLUTION_512x384;
    else if ((width == 640)  && (height == 400)) glideResolution = GR_RESOLUTION_640x400;
    else if ((width == 640)  && (height == 480)) glideResolution = GR_RESOLUTION_640x480;
    else if ((width == 800)  && (height == 600)) glideResolution = GR_RESOLUTION_800x600;
    else if ((width == 1024) && (height == 768)) glideResolution = GR_RESOLUTION_1024x768;
    else assert(0 && "Glide 3.x renderer: Unsupported render resolution.");

    shiet_window_win32__create_window(width, height, windowTitle);
    shiet_window_win32__get_window_handle((void*)&WINDOW_HANDLE);

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    WINDOW_DC = GetDC(WINDOW_HANDLE);

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);
    UpdateWindow(WINDOW_HANDLE);

    grGlideInit();
    grSstSelect(0);
    GLIDE_RENDER_CONTEXT = grSstWinOpen((FxU32)WINDOW_HANDLE,
                                        glideResolution,
                                        GR_REFRESH_60Hz,
                                        GR_COLORFORMAT_ARGB,
                                        GR_ORIGIN_UPPER_LEFT,
                                        2,
                                        1);
    
    assert(GLIDE_RENDER_CONTEXT &&
           "Glide 3.x renderer: Failed to initialize the renderer.");

    return;
}
