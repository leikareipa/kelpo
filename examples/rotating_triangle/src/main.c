/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Renders a simple rotating triangle using the shiet renderer.
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <shiet_interface/polygon/triangle/triangle_stack.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include <shiet_interface/interface.h>
#include <shiet_interface/common/globals.h>
#include "../../common_src/transform_and_rotate_triangles.h"
#include "../../common_src/parse_command_line.h"

int main(int argc, char *argv[])
{
    struct { unsigned width; unsigned height; } renderResolution = {640, 480};
    struct shiet_interface_s renderer = shiet_create_interface("opengl_1_2");

    struct shiet_polygon_triangle_stack_s *triangles = shiet_tristack_create(1);
    struct shiet_polygon_triangle_stack_s *transformedTriangles = shiet_tristack_create(triangles->capacity);

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

        renderer.initialize(renderResolution.width, renderResolution.height, windowTitle);

        trirot_initialize_screen_geometry(renderResolution.width, renderResolution.height);
    }

    /* Create the triangle that will be rendered in the middle of the screen.*/
    {
        struct shiet_polygon_triangle_s triangle;

        triangle.vertex[0].x = -0.5;
        triangle.vertex[0].y = -0.5;
        triangle.vertex[0].z = 0;
        triangle.vertex[0].w = 1;
        triangle.vertex[0].u = 0;
        triangle.vertex[0].v = 0;
        triangle.vertex[0].r = 255;
        triangle.vertex[0].g = 0;
        triangle.vertex[0].b = 0;

        triangle.vertex[1].x = 0.5;
        triangle.vertex[1].y = -0.5;
        triangle.vertex[1].z = 0;
        triangle.vertex[1].w = 1;
        triangle.vertex[1].u = 1;
        triangle.vertex[1].v = 0;
        triangle.vertex[1].r = 0;
        triangle.vertex[1].g = 255;
        triangle.vertex[1].b = 0;

        triangle.vertex[2].x = 0.5;
        triangle.vertex[2].y = 0.5;
        triangle.vertex[2].z = 0;
        triangle.vertex[2].w = 1;
        triangle.vertex[2].u = 1;
        triangle.vertex[2].v = 1;
        triangle.vertex[2].r = 0;
        triangle.vertex[2].g = 0;
        triangle.vertex[2].b = 255;
        
        triangle.texture = NULL;

        shiet_tristack_push_copy(triangles, &triangle);
    }

    /* Render.*/
    while (renderer.window.is_window_open())
    {
        shiet_tristack_clear(transformedTriangles);

        trirot_transform_and_rotate_triangles(triangles,
                                              transformedTriangles,
                                              0, 0, 1.5,
                                              0, 0.01, 0);

        renderer.rasterizer.clear_frame();
        renderer.rasterizer.draw_triangles(transformedTriangles->data, transformedTriangles->count);
        renderer.window.update_window();
    }

    shiet_tristack_free(triangles);
    shiet_tristack_free(transformedTriangles);

    return 0;
}
