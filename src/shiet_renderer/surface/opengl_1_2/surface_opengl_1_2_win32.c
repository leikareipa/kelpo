/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * OpenGL 1.2 render surface for the shiet renderer.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <shiet_renderer/surface/opengl_1_2/surface_opengl_1_2_win32.h>
#include <shiet_renderer/window/win32/window_win32.h>

#include <windows.h>
#include <gl/glu.h>
#include <gl/gl.h>

static HDC WINDOW_DC = 0;
static HGLRC RENDER_CONTEXT = 0;
static HWND WINDOW_HANDLE = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;

/* Call this function whenever the size of the OpenGL window changes.*/
static void resize_opengl_display(GLsizei width, GLsizei height)
{
    glLoadIdentity();
    glTranslatef(0, 0, -1);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, WINDOW_HEIGHT, 0);

    return;
}

/* Enable/disable vsync.*/
static void set_opengl_vsync_enabled(const int vsyncOn)
{
    typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC) (int interval);
    const char *const extensions = (char*)glGetString(GL_EXTENSIONS);

    if (strstr(extensions,"WGL_EXT_swap_control"))
    {
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        wglSwapIntervalEXT(vsyncOn);
    }
    else
    {
        fprintf(stderr, "OpenGL 1.2: This device does not allow vsync to be toggled on/off.");
    }

    return;
}

void shiet_surface_opengl_1_2_win32__release_surface(void)
{
    assert((WINDOW_HANDLE &&
            RENDER_CONTEXT) &&
           "OpenGL 1.2: Attempting to release the display surface before it has been acquired.");

    /* Return from fullscreen.*/
    ChangeDisplaySettings(NULL, 0);

    if (!wglMakeCurrent(NULL, NULL) ||
        !wglDeleteContext(RENDER_CONTEXT) ||
        !ReleaseDC(WINDOW_HANDLE, WINDOW_DC))
    {
        fprintf(stderr, "OpenGL 1.2: Failed to properly release the display surface.");
    }

    return;
}

void shiet_surface_opengl_1_2_win32__flip_surface(void)
{
    SwapBuffers(WINDOW_DC);

    return;
}

static LRESULT window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

void shiet_surface_opengl_1_2_win32__create_surface(const unsigned width,
                                                    const unsigned height)
{
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        16,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    /* Enter fullscreen.*/
    {
        DEVMODE dmScreenSettings;

        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = WINDOW_WIDTH;
        dmScreenSettings.dmPelsHeight = WINDOW_HEIGHT;
        dmScreenSettings.dmBitsPerPel = 16;
        dmScreenSettings.dmFields = (DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT);

        if (ChangeDisplaySettingsA(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            assert(0 && "Failed to obtain an OpenGL-compatible display.");
            return;
        }
    }

    shiet_window_win32__create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "", window_proc);

    WINDOW_HANDLE = (HWND)shiet_window_win32__get_window_handle();
    WINDOW_DC = GetDC(WINDOW_HANDLE);

    if (!WINDOW_HANDLE ||
        !WINDOW_DC ||
        !SetPixelFormat(WINDOW_DC, ChoosePixelFormat(WINDOW_DC, &pfd), &pfd) ||
        !(RENDER_CONTEXT = wglCreateContext(WINDOW_DC)) ||
        !wglMakeCurrent(WINDOW_DC, RENDER_CONTEXT))
    {
        assert(0 && "OpenGL 1.2: Failed to create the render surface.");
        return;
    }

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);
    resize_opengl_display(WINDOW_WIDTH, WINDOW_HEIGHT);
    set_opengl_vsync_enabled(1);
    UpdateWindow(WINDOW_HANDLE);

    return;
}
