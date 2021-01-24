#include <stdio.h>
#include <kelpo_renderer/surface/opengl_3_0/surface_opengl_3_0.h>
#include <kelpo_renderer/rasterizer/opengl_3_0/rasterizer_opengl_3_0.h>
#include <kelpo_renderer/window/win32/window_win32.h>
#include <kelpo_interface/interface.h>

static const char RENDERER_NAME[] = "OpenGL 3.0";
static const unsigned RENDERER_VERSION[3] = {KELPO_INTERFACE_VERSION_MAJOR,
                                             1,   /* Minor.*/
                                             0};  /* Patch.*/

static int initialize(const unsigned deviceId,
                      const unsigned screenWidth,
                      const unsigned screenHeight,
                      const unsigned screenBPP)
{
    kelpo_surface_opengl_3_0__create_surface(screenWidth, screenHeight, screenBPP, 1, deviceId);
    kelpo_rasterizer_opengl_3_0__initialize();

    return 1;
}

static int release(void)
{
    kelpo_surface_opengl_3_0__release_surface();
    kelpo_rasterizer_opengl_3_0__release();
    kelpo_window__release_window();

    return 1;
}

void export_interface(struct kelpo_interface_s *const interface)
{
    interface->window.open = initialize;
    interface->window.release = release;
    interface->window.is_open = kelpo_window__is_window_open;
    interface->window.process_messages = kelpo_window__process_window_messages;
    interface->window.flip_surface = kelpo_surface_opengl_3_0__flip_surface;
    interface->window.get_handle = kelpo_window__get_window_handle;
    interface->window.set_message_handler = kelpo_window__set_external_message_handler;

    interface->rasterizer.clear_frame = kelpo_rasterizer_opengl_3_0__clear_frame;
    interface->rasterizer.draw_triangles = kelpo_rasterizer_opengl_3_0__draw_triangles;
    interface->rasterizer.upload_texture = kelpo_rasterizer_opengl_3_0__upload_texture;
    interface->rasterizer.update_texture = kelpo_rasterizer_opengl_3_0__update_texture;
    interface->rasterizer.unload_textures = kelpo_rasterizer_opengl_3_0__unload_textures;

    interface->metadata.rendererName = RENDERER_NAME;
    interface->metadata.rendererVersionMajor = RENDERER_VERSION[0];
    interface->metadata.rendererVersionMinor = RENDERER_VERSION[1];
    interface->metadata.rendererVersionPatch = RENDERER_VERSION[2];

    return;
}
