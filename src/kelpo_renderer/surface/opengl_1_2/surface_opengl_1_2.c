/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * OpenGL 1.2 render surface for the Kelpo renderer.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <kelpo_renderer/surface/opengl_1_2/surface_opengl_1_2.h>
#include <kelpo_renderer/rasterizer/opengl_1_2/rasterizer_opengl_1_2.h>
#include <kelpo_renderer/window/win32/window_win32.h>
#include <kelpo_interface/error.h>

#include <windows.h>
#include <gl/glu.h>
#include <gl/gl.h>

static HDC WINDOW_DC = 0;
static HGLRC RENDER_CONTEXT = 0;
static HWND WINDOW_HANDLE = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static unsigned WINDOW_BIT_DEPTH = 0;

/* Call this function whenever the size of the OpenGL window changes.*/
static void resize_opengl_display(GLsizei width, GLsizei height)
{
    glLoadIdentity();
    glTranslatef(0, 0, -1);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, WINDOW_HEIGHT, 0);

    return;
}

static void set_opengl_vsync_enabled(const int vsyncOn)
{
    typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC) (int interval);
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

    if (!wglSwapIntervalEXT ||
        !wglSwapIntervalEXT(vsyncOn))
    {
        kelpo_error(KELPOERR_VSYNC_CONTROL_NOT_SUPPORTED);
    }

    return;
}

void kelpo_surface_opengl_1_2__release_surface(void)
{
    assert((WINDOW_HANDLE &&
            RENDER_CONTEXT) &&
           "Attempting to release the display surface before it has been acquired.");

    if (!wglMakeCurrent(NULL, NULL) ||
        !wglDeleteContext(RENDER_CONTEXT) ||
        !ReleaseDC(WINDOW_HANDLE, WINDOW_DC))
    {
        kelpo_error(KELPOERR_API_CALL_FAILED);
    }

    /* Return from fullscreen.*/
    ChangeDisplaySettings(NULL, 0);

    return;
}

void kelpo_surface_opengl_1_2__flip_surface(void)
{
    SwapBuffers(WINDOW_DC);

    return;
}

static LRESULT window_proc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_SIZE:
        {
            const unsigned newWidth = LOWORD(lParam);
            const unsigned newHeight = HIWORD(lParam);
            resize_opengl_display(newWidth, newHeight);
            
            break;
        }

        default: break;
    }

    return 0;
}

void kelpo_surface_opengl_1_2__create_surface(const unsigned width,
                                              const unsigned height,
                                              const unsigned bpp,
                                              const int vsyncEnabled,
                                              const unsigned deviceIdx)
{
    PIXELFORMATDESCRIPTOR pfd;

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    WINDOW_BIT_DEPTH = bpp;
    
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = (PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER);
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.cColorBits = 3;
    pfd.cDepthBits = 1;

    /* Enter fullscreen.*/
    {
        DEVMODE dmScreenSettings;

        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = WINDOW_WIDTH;
        dmScreenSettings.dmPelsHeight = WINDOW_HEIGHT;
        dmScreenSettings.dmBitsPerPel = WINDOW_BIT_DEPTH;
        dmScreenSettings.dmFields = (DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT);

        if (ChangeDisplaySettingsA(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            kelpo_error(KELPOERR_DISPLAY_MODE_NOT_SUPPORTED);
            return;
        }
    }

    kelpo_window__create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL 1.2", window_proc);

    WINDOW_HANDLE = (HWND)kelpo_window__get_window_handle();
    WINDOW_DC = GetDC(WINDOW_HANDLE);

    if (!WINDOW_HANDLE ||
        !WINDOW_DC ||
        !SetPixelFormat(WINDOW_DC, ChoosePixelFormat(WINDOW_DC, &pfd), &pfd) ||
        !(RENDER_CONTEXT = wglCreateContext(WINDOW_DC)) ||
        !wglMakeCurrent(WINDOW_DC, RENDER_CONTEXT))
    {
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return;
    }

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);
    resize_opengl_display(WINDOW_WIDTH, WINDOW_HEIGHT);
    set_opengl_vsync_enabled(vsyncEnabled? 1 : 0);
    UpdateWindow(WINDOW_HANDLE);

    return;
}
