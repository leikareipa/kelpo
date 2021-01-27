/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Renders a simple rotating triangle using the Kelpo renderer.
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <kelpo_auxiliary/misc.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_auxiliary/triangle_preparer.h>
#include <kelpo_auxiliary/matrix_44.h>
#include <kelpo_interface/error.h>
#include <kelpo_interface/interface.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include "../../common_src/default_window_message_handler.h"
#include "../../common_src/parse_command_line.h"

int main(int argc, char *argv[])
{
    const struct kelpo_interface_s *kelpo = NULL;

    struct kelpoa_generic_stack_s *triangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *worldSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *screenSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));

    struct kelpoa_matrix44_s clipSpaceMatrix;
    struct kelpoa_matrix44_s screenSpaceMatrix;

    /* Set up default rendering options, and parse the command-line to see if
     * the user has provided any overrides for them.*/
    struct cliparse_params_s cliArgs = {0};
    cliArgs.rendererName = "opengl_1_2";
    cliArgs.windowWidth = 1920;
    cliArgs.windowHeight = 1080;
    cliArgs.windowBPP = 32;
    cliparse_get_params(argc, argv, &cliArgs);

    /* Initialize Kelpo.*/
    if (!kelpo_create_interface(&kelpo, cliArgs.rendererName) ||
        !kelpo->window.open(cliArgs.renderDeviceIdx, cliArgs.windowWidth, cliArgs.windowHeight, cliArgs.windowBPP) ||
        !kelpo->window.set_message_handler(default_window_message_handler))
    {
        fprintf(stderr, "Failed to initialize Kelpo.\n");
        goto cleanup;
    }

    /* Create the triangle to be rendered.*/
    {
        struct kelpo_polygon_triangle_s triangle;

        memset(&triangle, 0, sizeof(triangle));

        triangle.flags.twoSided = 1;

        triangle.vertex[0].x = -1;
        triangle.vertex[0].y = -1;
        triangle.vertex[0].w = 1;
        triangle.vertex[0].r = 255;
        triangle.vertex[0].a = 255;

        triangle.vertex[1].x = 1;
        triangle.vertex[1].y = -1;
        triangle.vertex[1].w = 1;
        triangle.vertex[1].g = 255;
        triangle.vertex[1].a = 255;

        triangle.vertex[2].x = 1;
        triangle.vertex[2].y = 1;
        triangle.vertex[2].w = 1;
        triangle.vertex[2].b = 255;
        triangle.vertex[2].a = 255;

        kelpoa_generic_stack__push_copy(triangles, &triangle);
    }

    /* Initialize transformation matrices, for rotating the triangle.*/
    {
        kelpoa_matrix44__make_clip_space_matrix(&clipSpaceMatrix,
                                                KELPOA_DEG_TO_RAD(60),
                                                (cliArgs.windowWidth / (float)cliArgs.windowHeight),
                                                0.1, 100);

        kelpoa_matrix44__make_screen_space_matrix(&screenSpaceMatrix,
                                                  (cliArgs.windowWidth / 2.0f),
                                                  (cliArgs.windowHeight / 2.0f));
    }

    /* Render loop.*/
    while (kelpo->window.process_messages(),
           kelpo->window.is_open())
    {
        static float rotY = 0;

        /* Rotate the triangle and transform its vertices into screen space.*/
        kelpoa_generic_stack__clear(worldSpaceTriangles);
        kelpoa_generic_stack__clear(screenSpaceTriangles);
        kelpoa_triprepr__duplicate_triangles(triangles, worldSpaceTriangles);
        kelpoa_triprepr__rotate_triangles(worldSpaceTriangles, 0, (rotY += 0.01), 0);
        kelpoa_triprepr__translate_triangles(worldSpaceTriangles, 0, 0, 3);
        kelpoa_triprepr__project_triangles_to_screen(worldSpaceTriangles,
                                                     screenSpaceTriangles,
                                                     &clipSpaceMatrix,
                                                     &screenSpaceMatrix,
                                                     0.1, 100, 1);

        /* Render the triangle.*/
        kelpo->rasterizer.clear_frame();
        kelpo->rasterizer.draw_triangles(screenSpaceTriangles->data, screenSpaceTriangles->count);
        kelpo->window.flip_surface();

        if (kelpo_error_peek() != KELPOERR_ALL_GOOD)
        {
            fprintf(stderr, "Kelpo has reported an error.\n");
            goto cleanup;
        }
    }

    cleanup:

    kelpoa_generic_stack__free(triangles);
    kelpoa_generic_stack__free(worldSpaceTriangles);
    kelpoa_generic_stack__free(screenSpaceTriangles);

    if (!kelpo_release_interface(kelpo))
    {
        fprintf(stderr, "Failed to release Kelpo.\n");
    }

    return (kelpo_error_peek() == KELPOERR_ALL_GOOD)
           ? EXIT_SUCCESS
           : EXIT_FAILURE;
}
