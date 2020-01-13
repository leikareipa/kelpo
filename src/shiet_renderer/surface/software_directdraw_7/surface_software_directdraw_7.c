/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * OpenGL 1.2 render surface for the shiet renderer.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <shiet_renderer/surface/software_directdraw_7/surface_software_directdraw_7.h>
#include <shiet_renderer/surface/directdraw_7/enumerate_directdraw_7_devices.h>
#include <shiet_renderer/surface/directdraw_7/surface_directdraw_7.h>
#include <shiet_renderer/window/window_win32.h>

#include <windows.h>

static HWND WINDOW_HANDLE = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;

void shiet_surface_software_directdraw_7__release_surface(void)
{
    return;
}

void shiet_surface_software_directdraw_7__flip_surface(void)
{
    shiet_surface_directdraw_7__flip_surface();

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

void shiet_surface_software_directdraw_7__create_surface(const unsigned width,
                                                               const unsigned height)
{
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;

    shiet_window__create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "", window_proc);
    WINDOW_HANDLE = (HWND)shiet_window__get_window_handle();

    if (!WINDOW_HANDLE)
    {
        assert(0 && "Software w/ DirectDraw 7: Failed to create the render surface.");
        return;
    }

    shiet_surface_directdraw_7__initialize_surface(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_HANDLE,
                                                         shiet_directdraw7_device_guid(0));

    ShowWindow(WINDOW_HANDLE, SW_SHOW);
    SetForegroundWindow(WINDOW_HANDLE);
    SetFocus(WINDOW_HANDLE);

    return;
}
