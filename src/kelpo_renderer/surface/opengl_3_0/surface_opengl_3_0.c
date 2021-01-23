/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 * OpenGL 3.0 render surface for the Kelpo renderer.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <kelpo_renderer/surface/opengl_3_0/surface_opengl_3_0.h>
#include <kelpo_renderer/rasterizer/opengl_3_0/rasterizer_opengl_3_0.h>
#include <kelpo_renderer/window/win32/window_win32.h>

#include <windows.h>
#include <gl/glu.h>
#include <gl/gl.h>
#include <gl/wglext.h>
#include <kelpo_renderer/surface/opengl_3_0/gl3_types.h>

static HDC WINDOW_DC = 0;
static HGLRC RENDER_CONTEXT = 0;
static HWND WINDOW_HANDLE = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static unsigned WINDOW_BIT_DEPTH = 0;

/* Will contain the output of glGetString(GL_EXTENSIONS) and
 * wglGetExtensionsStringARB(WINDOW_DC), respectively, as queried on
 * surface initialization.*/
static const char *GL_EXTENSIONS_STRING = NULL;
static const char *WGL_EXTENSIONS_STRING = NULL;

/* The OpenGL extensions we're going to be using.*/
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
static PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

static int is_ext_available(const char *const extensionName)
{
    assert(GL_EXTENSIONS_STRING &&
           WGL_EXTENSIONS_STRING
           && "OpenGL 3.0: Extensions string not initialized.");

    return (strstr(GL_EXTENSIONS_STRING, extensionName) ||
            strstr(WGL_EXTENSIONS_STRING, extensionName));
}

/* Returns 1 if successful; 0 otherwise.*/
static int initialize_extensions(void)
{
    assert(WINDOW_DC && "OpenGL 3.0: The device context has not been initialized.\n");

    /* Establish a list of available extensions.*/
    {
        wglGetExtensionsStringARB =
            (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");

        if (!wglGetExtensionsStringARB ||
            !(GL_EXTENSIONS_STRING = (char*)glGetString(GL_EXTENSIONS)) ||
            !(WGL_EXTENSIONS_STRING = wglGetExtensionsStringARB(WINDOW_DC)))
        {
            fprintf(stderr, "OpenGL 3.0: The required extension \"WGL_ARB_extensions_string\" is not supported by this hardware.\n");
            return 0;
        }
    }

    /* Get pointers to the extensions we're going to use.*/
    {
        wglSwapIntervalEXT =
            (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

        glCreateProgram =
            (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");

        glCreateShader =
            (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
            
        glShaderSource =
            (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
            
        glCompileShader =
            (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
            
        glGetShaderiv =
            (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
            
        glAttachShader =
            (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
            
        glBindFragDataLocation =
            (PFNGLBINDFRAGDATALOCATIONPROC)wglGetProcAddress("glBindFragDataLocation");
            
        glLinkProgram =
            (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
            
        glUseProgram =
            (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
            
        glGenBuffers =
            (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
            
        glBindBuffer =
            (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
            
        glBufferData =
            (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");

        glBufferSubData =
            (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
            
        glGenVertexArrays =
            (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
            
        glBindVertexArray =
            (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
            
        glGetAttribLocation =
            (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
            
        glVertexAttribPointer =
            (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
            
        glEnableVertexAttribArray =
            (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
            
    }

    return 1;
}

/* Call this function whenever the size of the OpenGL window changes.*/
static void resize_opengl_display(GLsizei width, GLsizei height)
{
    glViewport(0, 0, width, height);

    glLoadIdentity();
    glTranslatef(0, 0, -1);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, 0, height, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, height, 0);

    return;
}

static void set_vsync_enabled(const int vsyncOn)
{
    if (wglSwapIntervalEXT)
    {
        if (!wglSwapIntervalEXT(vsyncOn))
        {
            fprintf(stderr, "OpenGL 3.0: Failed to %s vsync.\n", (vsyncOn? "enable" : "disable"));
        }
    }
    else
    {
        fprintf(stderr, "OpenGL 3.0: This hardware does not support vsync control.\n");
    }

    return;
}

void kelpo_surface_opengl_3_0__release_surface(void)
{
    assert((WINDOW_HANDLE &&
            RENDER_CONTEXT) &&
           "OpenGL 3.0: Attempting to release the display surface before it has been acquired.");

    kelpo_rasterizer_opengl_3_0__release();

    kelpo_window__release_window();

    /* Return from fullscreen.*/
    ChangeDisplaySettings(NULL, 0);

    if (!wglMakeCurrent(NULL, NULL) ||
        !wglDeleteContext(RENDER_CONTEXT) ||
        !ReleaseDC(WINDOW_HANDLE, WINDOW_DC))
    {
        fprintf(stderr, "OpenGL 3.0: Failed to properly release the display surface.");
    }

    return;
}

void kelpo_surface_opengl_3_0__flip_surface(void)
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

/* Returns 1 if successful; 0 otherwise.*/
static int create_opengl_3_context(void)
{
    HGLRC dummyContext = NULL;

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;

    int contextAttributes[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 0,
        WGL_CONTEXT_FLAGS_ARB, 0,
        0
    };

    assert(WINDOW_DC && "OpenGL 3.0: The window's device context has not been initialized.");

    if (!(dummyContext = wglCreateContext(WINDOW_DC)) ||
        !wglMakeCurrent(WINDOW_DC, dummyContext))
    {
        fprintf(stderr, "OpenGL 3.0: Failed to create a temporary render context.");
        return 0;
    }

    /* Note: A current render context must be established before this.*/
    wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglCreateContextAttribsARB ||
        !(RENDER_CONTEXT = wglCreateContextAttribsARB(WINDOW_DC, 0, contextAttributes)))
    {
        fprintf(stderr, "OpenGL 3.0: The required extension \"WGL_ARB_create_context\" is not supported by this hardware.");
        return 0;
    }

    if (!wglMakeCurrent(NULL, NULL) ||
        !wglDeleteContext(dummyContext) ||
        !wglMakeCurrent(WINDOW_DC, RENDER_CONTEXT))
    {
        fprintf(stderr, "OpenGL 3.0: Failed to create the render context.");

        return 0;
    }

    return 1;
}

void kelpo_surface_opengl_3_0__create_surface(const unsigned width,
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
            assert(0 && "Failed to obtain an OpenGL-compatible display.");
        }
    }

    kelpo_window__create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL 3.3", window_proc);

    if (!(WINDOW_HANDLE = (HWND)kelpo_window__get_window_handle()) ||
        !(WINDOW_DC = GetDC(WINDOW_HANDLE)) ||
        !SetPixelFormat(WINDOW_DC, ChoosePixelFormat(WINDOW_DC, &pfd), &pfd))
    {
        assert(0 && "OpenGL 3.0: Could not create an OpenGL-compatible window.");
    }

    if (!create_opengl_3_context() ||
        !initialize_extensions())
    {
        assert(0 && "OpenGL 3.0: Could not create an OpenGL 3.0 render context.");
    }

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);
    resize_opengl_display(WINDOW_WIDTH, WINDOW_HEIGHT);
    set_vsync_enabled(vsyncEnabled? 1 : 0);
    UpdateWindow(WINDOW_HANDLE);

    return;
}
