/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Renders a simple rotating triangle using the shiet renderer.
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <shiet_interface/generic_data_stack.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include <shiet_interface/interface.h>
#include <shiet_interface/common/globals.h>
#include "../../common_src/transform_and_rotate_triangles.h"
#include "../../common_src/parse_command_line.h"

#include <windows.h>

int main(int argc, char *argv[])
{
    /* An index in an enumeration of API-compatible devices on the system,
     * identifying the devide to be used in rendering.*/
    unsigned renderDeviceIdx = 0;

    /* If set to 1, we'll request the renderer to use vsync. Otherwise, we'll
     * ask for vsync to be off. On some hardware, this option will have no
     * effect, however.*/
    unsigned vsyncEnabled = 1;

    struct { unsigned width; unsigned height; unsigned bpp; } renderResolution = {640, 480, 16};
    struct shiet_interface_s renderer = shiet_create_interface("opengl_1_2");

    struct shiet_generic_data_stack_s *triangles = shiet_generic_data_stack__create(1, sizeof(struct shiet_polygon_triangle_s));
    struct shiet_generic_data_stack_s *transformedTriangles = shiet_generic_data_stack__create(triangles->capacity, sizeof(struct shiet_polygon_triangle_s));

    /* Process any relevant command-line parameters.*/
    {
        int c = 0;
        while ((c = shiet_cliparse(argc, argv)) != -1)
        {
            switch (c)
            {
                case 'r':
                {
                    renderer = shiet_create_interface(shiet_cliparse_optarg());
                    break;
                }
                case 'v':
                {
                    vsyncEnabled = strtoul(shiet_cliparse_optarg(), NULL, 10);
                    break;
                }
                case 'w':
                {
                    renderResolution.width = strtoul(shiet_cliparse_optarg(), NULL, 10);
                    assert((renderResolution.width != 0u) && "Invalid render width.");
                    break;
                }
                case 'h':
                {
                    renderResolution.height = strtoul(shiet_cliparse_optarg(), NULL, 10);
                    assert((renderResolution.height != 0u) && "Invalid render height.");
                    break;
                }
                case 'b':
                {
                    renderResolution.bpp = strtoul(shiet_cliparse_optarg(), NULL, 10);
                    assert((renderResolution.bpp != 0u) && "Invalid render bit depth.");
                    break;
                }
                case 'd':
                {
                    /* The device index is expected to be 1-indexed (device #1 is 1).*/
                    renderDeviceIdx = strtoul(shiet_cliparse_optarg(), NULL, 10);
                    assert((renderDeviceIdx != 0u) && "Invalid render device index.");
                    renderDeviceIdx--;
                    break;
                }
                default: break;
            }
        }
    }

    /* Initialize the renderer.*/
    {
        char windowTitle[128];

        sprintf(windowTitle, "shiet %d.%d.%d / %s (%d.%d.%d)",
                SHIET_INTERFACE_VERSION_MAJOR,
                SHIET_INTERFACE_VERSION_MINOR,
                SHIET_INTERFACE_VERSION_PATCH,
                renderer.metadata.rendererName,
                renderer.metadata.rendererVersionMajor,
                renderer.metadata.rendererVersionMinor,
                renderer.metadata.rendererVersionPatch);

        renderer.initialize(renderResolution.width,
                            renderResolution.height,
                            renderResolution.bpp,
                            vsyncEnabled,
                            renderDeviceIdx);

        SetWindowTextA((HWND)renderer.window.get_handle(), windowTitle);

        trirot_initialize_screen_geometry(renderResolution.width, renderResolution.height);
    }

    /* Create the triangle that will be rendered in the middle of the screen.*/
    {
        struct shiet_polygon_triangle_s triangle;

        memset(&triangle, 0, sizeof(triangle));

        triangle.vertex[0].x = -1;
        triangle.vertex[0].y = -1;
        triangle.vertex[0].z = 0;
        triangle.vertex[0].w = 1;
        triangle.vertex[0].u = 0;
        triangle.vertex[0].v = 0;
        triangle.vertex[0].nx = 0;
        triangle.vertex[0].ny = 0;
        triangle.vertex[0].nz = -1;
        triangle.vertex[0].r = 255;
        triangle.vertex[0].g = 0;
        triangle.vertex[0].b = 0;
        triangle.vertex[0].a = 255;

        triangle.vertex[1].x = 1;
        triangle.vertex[1].y = -1;
        triangle.vertex[1].z = 0;
        triangle.vertex[1].w = 1;
        triangle.vertex[1].u = 1;
        triangle.vertex[1].v = 0;
        triangle.vertex[1].nx = 0;
        triangle.vertex[1].ny = 0;
        triangle.vertex[1].nz = -1;
        triangle.vertex[1].r = 0;
        triangle.vertex[1].g = 255;
        triangle.vertex[1].b = 0;
        triangle.vertex[1].a = 255;

        triangle.vertex[2].x = 1;
        triangle.vertex[2].y = 1;
        triangle.vertex[2].z = 0;
        triangle.vertex[2].w = 1;
        triangle.vertex[2].u = 1;
        triangle.vertex[2].v = 1;
        triangle.vertex[2].nx = 0;
        triangle.vertex[2].ny = 0;
        triangle.vertex[2].nz = -1;
        triangle.vertex[2].r = 0;
        triangle.vertex[2].g = 0;
        triangle.vertex[2].b = 255;
        triangle.vertex[2].a = 255;
        
        shiet_generic_data_stack__push_copy(triangles, &triangle);
    }

    /* Render.*/
    while (renderer.window.is_window_open())
    {
        renderer.window.process_events();

        shiet_generic_data_stack__clear(transformedTriangles);
        trirot_transform_and_rotate_triangles(triangles,
                                              transformedTriangles,
                                              0, 0, 3,
                                              0, 0.01, 0);

        renderer.rasterizer.clear_frame();
        renderer.rasterizer.draw_triangles(transformedTriangles->data,
                                           transformedTriangles->count);

        renderer.window.flip_surface();
    }

    shiet_generic_data_stack__free(triangles);
    shiet_generic_data_stack__free(transformedTriangles);

    return 0;
}
