/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Renders a simple static triangle using the Kelpo renderer.
 * 
 */

#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/interface.h>
#include "../../common_src/default_window_message_handler.h"
#include "../../common_src/parse_command_line.h"

int main(int argc, char *argv[])
{
    struct kelpo_interface_s renderer = {0};
    struct kelpo_cliparse_params_s cliParams = {0};
    struct kelpo_polygon_triangle_s triangle;

    /* Set up default rendering options, and parse the command-line to see if
     * the user has provided any overrides for them.*/
    {
        cliParams.rendererName = "opengl_1_2";
        cliParams.windowWidth = 1920;
        cliParams.windowHeight = 1080;
        cliParams.windowBPP = 32;

        kelpo_cliparse_get_params(argc, argv, &cliParams);
    }

    /* Create the triangle that will be rendered in the middle of the screen.*/
    {
        const int widthHalf = (cliParams.windowWidth / 2);
        const int heightHalf = (cliParams.windowHeight / 2);
        const int widthQuarter = (widthHalf / 2);
        const int heightQuarter = (heightHalf / 2);

        memset(&triangle, 0, sizeof(triangle));

        triangle.vertex[0].x = widthQuarter;
        triangle.vertex[0].y = heightQuarter;
        triangle.vertex[0].z = 1;
        triangle.vertex[0].r = 255;
        triangle.vertex[0].a = 255;

        triangle.vertex[1].x = (widthQuarter + widthHalf);
        triangle.vertex[1].y = heightQuarter;
        triangle.vertex[1].z = 1;
        triangle.vertex[1].g = 255;
        triangle.vertex[1].a = 255;

        triangle.vertex[2].x = (widthQuarter + widthHalf);
        triangle.vertex[2].y = (heightQuarter + heightHalf);
        triangle.vertex[2].z = 1;
        triangle.vertex[2].b = 255;
        triangle.vertex[2].a = 255;
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

    /* Render the triangle.*/
    while (renderer.window.is_open())
    {
        renderer.window.process_messages();

        renderer.rasterizer.clear_frame();
        renderer.rasterizer.draw_triangles(&triangle, 1);

        renderer.window.flip_surface();
    }

    return 0;
}
