/*
 * Based loosely on Jeff 'NeHe' Molofee's Win32 window-creation routines.
 *
 */

#include <assert.h>
#include <windows.h>
#include <gl/glu.h>
#include <gl/gl.h>
#include "shiet_lib/render/rasterizer/opengl/surface_opengl.h"
#include "shiet_lib/render/window/win32/window_win32.h"

static HDC WINDOW_DC = 0;
static HGLRC RENDER_CONTEXT = 0;
static HWND WINDOW_HANDLE = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;

static void resize_gl(GLsizei width, GLsizei height)
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

static void initialize_gl_state(void)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glAlphaFunc(GL_GREATER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    return;
}

static void set_gl_vsync(const int vsyncOn)
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
		/* TODO: Handle vsync not supported by video card.*/
	}

	return;
}

void shiet_surface_opengl__release_surface(void)
{
    return;
}

void shiet_surface_opengl__update_surface(void)
{
    MSG m;

    InvalidateRect(WINDOW_HANDLE, NULL, FALSE);

    while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }

    SwapBuffers(WINDOW_DC);

    return;
}

void shiet_surface_opengl__create_surface(const unsigned width,
                                          const unsigned height,
                                          const char *const windowTitle)
{
    GLuint pixelFormat;

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

    shiet_window_win32__create_window(width, height, windowTitle);
    shiet_window_win32__get_window_handle((void*)&WINDOW_HANDLE);

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    WINDOW_DC = GetDC(WINDOW_HANDLE);
    pixelFormat = ChoosePixelFormat(WINDOW_DC, &pfd);

    if (!SetPixelFormat(WINDOW_DC, pixelFormat, &pfd))
    {
        assert(0 && "Failed to obtain an OpenGL-compatible display.");
        return;
    }
    RENDER_CONTEXT = wglCreateContext(WINDOW_DC);
    if (!wglMakeCurrent(WINDOW_DC, RENDER_CONTEXT))
    {
        assert(0 && "Failed to obtain an OpenGL-compatible display.");
        return;
    }

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);

    resize_gl(WINDOW_WIDTH, WINDOW_HEIGHT);
    initialize_gl_state();
	set_gl_vsync(1);

    UpdateWindow(WINDOW_HANDLE);

    return;
}
