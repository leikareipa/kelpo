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

int main(int argc, char *argv[])
{
    /* The location, in world units, of the near and far clipping planes.*/
    const float zNear = 0.1;
    const float zFar = 100;

    struct kelpoa_generic_stack_s *triangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *worldSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *screenSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));

    struct kelpoa_matrix44_s clipSpaceMatrix;
    struct kelpoa_matrix44_s screenSpaceMatrix;

    struct kelpo_interface_s renderer;

    /* Set up default rendering options, and parse the command-line to see if
     * the user has provided any overrides for them.*/
    struct cliparse_params_s cliParams = {0};
    {
        cliParams.rendererName = "opengl_1_2";
        cliParams.windowWidth = 1920;
        cliParams.windowHeight = 1080;
        cliParams.windowBPP = 32;

        cliparse_get_params(argc, argv, &cliParams);
    }

    /* Initialize the renderer.*/
    {
        renderer = kelpo_create_interface(cliParams.rendererName);

        renderer.window.open(cliParams.renderDeviceIdx,
                             cliParams.windowWidth,
                             cliParams.windowHeight,
                             cliParams.windowBPP);

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

        triangle.flags.twoSided = 1;
        
        kelpoa_generic_stack__push_copy(triangles, &triangle);
    }

    kelpoa_matrix44__make_clip_space_matrix(&clipSpaceMatrix,
                                            KELPOA_DEG_TO_RAD(60),
                                            (cliParams.windowWidth / (float)cliParams.windowHeight),
                                            zNear,
                                            zFar);

    kelpoa_matrix44__make_screen_space_matrix(&screenSpaceMatrix,
                                              (cliParams.windowWidth / 2.0f),
                                              (cliParams.windowHeight / 2.0f));

    /* Render.*/
    while (renderer.window.is_open())
    {
        static float rotX = 0, rotY = 0, rotZ = 0;
        rotY += 0.01;

        renderer.window.process_messages();

        /* Transform the scene's triangles into screen space.*/
        kelpoa_generic_stack__clear(worldSpaceTriangles);
        kelpoa_generic_stack__clear(screenSpaceTriangles);
        kelpoa_triprepr__duplicate_triangles(triangles, worldSpaceTriangles);
        kelpoa_triprepr__rotate_triangles(worldSpaceTriangles, rotX, rotY, rotZ);
        kelpoa_triprepr__translate_triangles(worldSpaceTriangles, 0, 0, 3);
        kelpoa_triprepr__project_triangles_to_screen(worldSpaceTriangles,
                                                     screenSpaceTriangles,
                                                     &clipSpaceMatrix,
                                                     &screenSpaceMatrix,
                                                     zNear,
                                                     zFar,
                                                     1);

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
