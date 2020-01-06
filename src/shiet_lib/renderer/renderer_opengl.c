#include <stdio.h>
#include <shiet_lib/renderer/rasterizer/opengl/surface_opengl_win32.h>
#include <shiet_lib/renderer/rasterizer/opengl/rasterizer_opengl.h>
#include <shiet_lib/renderer/window/win32/window_win32.h>
#include <shiet_interface/interface.h>

static const char RENDERER_NAME[] = "OpenGL";
static const unsigned RENDERER_VERSION[3] = {SHIET_INTERFACE_VERSION_MAJOR,
                                             1,   /* Minor.*/
                                             0};  /* Patch.*/

static void initialize_renderer(const unsigned windowWidth,
                                const unsigned windowHeight,
                                const char *const windowTitle)
{
    shiet_surface_opengl_win32__create_surface(windowWidth, windowHeight, windowTitle);
    shiet_rasterizer_opengl__initialize();

    return;
}

void import_renderer(struct shiet_interface_s *const interface)
{
    interface->initialize = initialize_renderer;

    interface->window.is_window_open = shiet_window_win32__is_window_open;
    interface->window.update_window = shiet_surface_opengl_win32__update_surface;
    interface->window.get_handle = shiet_window_win32__get_window_handle;

    interface->rasterizer.clear_frame = shiet_rasterizer_opengl__clear_frame;
    interface->rasterizer.draw_triangles = shiet_rasterizer_opengl__draw_triangles;
    interface->rasterizer.upload_texture = shiet_rasterizer_opengl__upload_texture;
    interface->rasterizer.update_texture = shiet_rasterizer_opengl__update_texture;

    interface->metadata.rendererName = RENDERER_NAME;
    interface->metadata.rendererVersionMajor = RENDERER_VERSION[0];
    interface->metadata.rendererVersionMinor = RENDERER_VERSION[1];
    interface->metadata.rendererVersionPatch = RENDERER_VERSION[2];

    return;
}
