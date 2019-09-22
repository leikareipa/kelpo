#include <stdio.h>
#include "shiet_lib/render/rasterizer/opengl/rasterizer_opengl.h"
#include "shiet_lib/render/rasterizer/opengl/surface_opengl.h"
#include "shiet_lib/render/window/win32/window_win32.h"
#include "shiet/interface.h"

static void shiet_render_opengl__initialize(const unsigned windowWidth,
                                            const unsigned windowHeight,
                                            const char *const windowTitle)
{
    shiet_surface_opengl__create_surface(windowWidth, windowHeight, windowTitle);

    return;
}

void shiet_render__get_function_pointers(struct shiet_render_interface_s *const interface)
{
    interface->window.is_window_open = shiet_window_win32__is_window_open;
    interface->window.update_window = shiet_surface_opengl__update_surface;
    interface->window.get_handle = shiet_window_win32__get_window_handle;

    interface->rasterizer.clear_frame = shiet_rasterizer_opengl__clear_frame;
    interface->rasterizer.draw_triangles = shiet_rasterizer_opengl__draw_triangles;

    interface->initialize = shiet_render_opengl__initialize;

    return;
}
