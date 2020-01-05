#include <stdio.h>
#include <shiet_lib/renderer/rasterizer/glide3/surface_glide3_win32.h>
#include <shiet_lib/renderer/rasterizer/glide3/rasterizer_glide3.h>
#include <shiet_lib/renderer/window/win32/window_win32.h>
#include <shiet/renderer_interface.h>

static const char RENDERER_NAME[] = "Glide 3.x";
static const unsigned RENDERER_VERSION[3] = {0, 0, 1}; /* Major, minor, patch.*/

static void initialize_renderer(const unsigned windowWidth,
                                const unsigned windowHeight,
                                const char *const windowTitle)
{
    shiet_surface_glide3_win32__create_surface(windowWidth, windowHeight, windowTitle);
    shiet_rasterizer_glide3__initialize();

    return;
}

void shiet_renderer_glide3__set_function_pointers(struct shiet_renderer_interface_s *const interface)
{
    interface->initialize = initialize_renderer;

    interface->window.is_window_open = shiet_window_win32__is_window_open;
    interface->window.update_window = shiet_surface_glide3_win32__update_surface;
    interface->window.get_handle = shiet_window_win32__get_window_handle;

    interface->rasterizer.clear_frame = shiet_rasterizer_glide3__clear_frame;
    interface->rasterizer.draw_triangles = shiet_rasterizer_glide3__draw_triangles;
    interface->rasterizer.upload_texture = shiet_rasterizer_glide3__upload_texture;
    interface->rasterizer.update_texture = shiet_rasterizer_glide3__update_texture;

    interface->metadata.rendererName = RENDERER_NAME;
    interface->metadata.rendererMajorVersion = RENDERER_VERSION[0];
    interface->metadata.rendererMinorVersion = RENDERER_VERSION[1];
    interface->metadata.rendererPatchVersion = RENDERER_VERSION[2];

    return;
}