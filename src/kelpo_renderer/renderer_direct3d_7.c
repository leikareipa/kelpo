#include <stdio.h>
#include <kelpo_renderer/surface/direct3d_7/surface_direct3d_7.h>
#include <kelpo_renderer/rasterizer/direct3d_7/rasterizer_direct3d_7.h>
#include <kelpo_renderer/window/window_win32.h>
#include <kelpo_interface/interface.h>

static const char RENDERER_NAME[] = "Direct3D 7";
static const unsigned RENDERER_VERSION[3] = {KELPO_INTERFACE_VERSION_MAJOR,
                                             0,   /* Minor.*/
                                             1};  /* Patch.*/

static void initialize_renderer(const unsigned windowWidth,
                                const unsigned windowHeight,
                                const unsigned bpp,
                                const int vsyncEnabled,
                                const unsigned deviceID)
{
    kelpo_surface_direct3d_7__create_surface(windowWidth, windowHeight, bpp, vsyncEnabled, deviceID);
    kelpo_rasterizer_direct3d_7__initialize();

    return;
}

#ifdef __cplusplus
extern "C"
{
#endif

void import_renderer(struct kelpo_interface_s *const interface)
{
    interface->initialize = initialize_renderer;

    interface->window.is_window_open = kelpo_window__is_window_open;
    interface->window.process_events = kelpo_window__process_window_events;
    interface->window.flip_surface = kelpo_surface_direct3d_7__flip_surface;
    interface->window.get_handle = kelpo_window__get_window_handle;

    interface->rasterizer.clear_frame = kelpo_rasterizer_direct3d_7__clear_frame;
    interface->rasterizer.draw_triangles = kelpo_rasterizer_direct3d_7__draw_triangles;
    interface->rasterizer.upload_texture = kelpo_rasterizer_direct3d_7__upload_texture;
    interface->rasterizer.update_texture = kelpo_rasterizer_direct3d_7__update_texture;
    interface->rasterizer.purge_textures = kelpo_rasterizer_direct3d_7__purge_textures;

    interface->metadata.rendererName = RENDERER_NAME;
    interface->metadata.rendererVersionMajor = RENDERER_VERSION[0];
    interface->metadata.rendererVersionMinor = RENDERER_VERSION[1];
    interface->metadata.rendererVersionPatch = RENDERER_VERSION[2];

    return;
}

#ifdef __cplusplus
}
#endif
