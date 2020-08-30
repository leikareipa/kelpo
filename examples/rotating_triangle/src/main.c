/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Renders a simple rotating triangle using the Kelpo renderer.
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_auxiliary/misc.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_auxiliary/triangle_preparer.h>
#include <kelpo_auxiliary/matrix_44.h>
#include <kelpo_interface/interface.h>
#include "../../common_src/default_window_message_handler.h"
#include "../../common_src/parse_command_line.h"

#include <windows.h>

/* The default render resolution. Note: The render resolution may be modified
 * by command-line arguments.*/
static struct { unsigned width; unsigned height; unsigned bpp; } RENDER_RESOLUTION = {640, 480, 16};

int main(int argc, char *argv[])
{
    /* An index in an enumeration of API-compatible devices on the system,
     * identifying the devide to be used in rendering.*/
    unsigned renderDeviceIdx = 0;

    /* If set to 1, we'll request the renderer to use vsync. Otherwise, we'll
     * ask for vsync to be off. On some hardware, this option will have no
     * effect, however.*/
    unsigned vsyncEnabled = 1;

    struct kelpo_interface_s renderer = kelpo_create_interface("opengl_1_2");

    struct kelpoa_generic_stack_s *triangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *worldSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *screenSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));

    struct kelpoa_matrix44_s clipSpaceMatrix;
    struct kelpoa_matrix44_s screenSpaceMatrix;

    /* Process any relevant command-line parameters.*/
    {
        int c = 0;
        while ((c = kelpo_cliparse(argc, argv)) != -1)
        {
            switch (c)
            {
                case 'r':
                {
                    renderer = kelpo_create_interface(kelpo_cliparse_optarg());
                    break;
                }
                case 'v':
                {
                    vsyncEnabled = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    break;
                }
                case 'w':
                {
                    RENDER_RESOLUTION.width = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    assert((RENDER_RESOLUTION.width != 0u) && "Invalid render width.");
                    break;
                }
                case 'h':
                {
                    RENDER_RESOLUTION.height = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    assert((RENDER_RESOLUTION.height != 0u) && "Invalid render height.");
                    break;
                }
                case 'b':
                {
                    RENDER_RESOLUTION.bpp = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    assert((RENDER_RESOLUTION.bpp != 0u) && "Invalid render bit depth.");
                    break;
                }
                case 'd':
                {
                    /* The device index is expected to be 1-indexed (device #1 is 1).*/
                    renderDeviceIdx = strtoul(kelpo_cliparse_optarg(), NULL, 10);
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
        renderer.initialize(RENDER_RESOLUTION.width,
                            RENDER_RESOLUTION.height,
                            RENDER_RESOLUTION.bpp,
                            vsyncEnabled,
                            renderDeviceIdx);

        renderer.window.set_message_handler(default_window_message_handler);
    }

    /* Create the triangle that will be rendered in the middle of the screen.*/
    {
        struct kelpo_polygon_triangle_s triangle;

        memset(&triangle, 0, sizeof(triangle));

        triangle.texture = NULL;

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
        
        kelpoa_generic_stack__push_copy(triangles, &triangle);
    }

    kelpoa_matrix44__make_clip_space_matrix(&clipSpaceMatrix,
                                            KELPOA_DEG_TO_RAD(60),
                                            (RENDER_RESOLUTION.width / (float)RENDER_RESOLUTION.height),
                                            0.1,
                                            100);

    kelpoa_matrix44__make_screen_space_matrix(&screenSpaceMatrix,
                                              (RENDER_RESOLUTION.width / 2.0f),
                                              (RENDER_RESOLUTION.height / 2.0f));

    /* Render.*/
    while (renderer.window.is_open())
    {
        static float rotX = 0, rotY = 0, rotZ = 0;
        rotY += 0.01;

        renderer.window.process_events();

        /* Transform the scene's triangles into screen space.*/
        kelpoa_generic_stack__clear(worldSpaceTriangles);
        kelpoa_generic_stack__clear(screenSpaceTriangles);
        kelpoa_triprepr__duplicate_triangles(triangles, worldSpaceTriangles);
        kelpoa_triprepr__rotate_triangles(worldSpaceTriangles, rotX, rotY, rotZ);
        kelpoa_triprepr__translate_triangles(worldSpaceTriangles, 0, 0, 3);
        kelpoa_triprepr__project_triangles_to_screen(worldSpaceTriangles, screenSpaceTriangles, &clipSpaceMatrix, &screenSpaceMatrix, 0);

        renderer.rasterizer.clear_frame();
        renderer.rasterizer.draw_triangles(screenSpaceTriangles->data,
                                           screenSpaceTriangles->count);

        renderer.window.flip_surface();
    }

    kelpoa_generic_stack__free(triangles);
    kelpoa_generic_stack__free(worldSpaceTriangles);
    kelpoa_generic_stack__free(screenSpaceTriangles);

    return 0;
}
