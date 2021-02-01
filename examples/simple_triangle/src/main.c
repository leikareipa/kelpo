/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Renders a simple static triangle using the Kelpo renderer.
 * 
 */

#include <stdio.h>
#include <windows.h> /* For the window message handler.*/
#include <kelpo_interface/error.h>
#include <kelpo_interface/interface.h>
#include <kelpo_interface/polygon/triangle/triangle.h>

LRESULT window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    /* Close the Kelpo window on Esc. We'll keep testing for kelpo.window.is_open()
     * in our rendering loop, and shut down once the window is no longer open.*/
    if ((message == WM_KEYDOWN) &&
        (wParam == VK_ESCAPE))
    {
        PostMessage(windowHandle, WM_CLOSE, 0, 0);
        return 1;
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    const struct kelpo_interface_s *kelpo = NULL;
    struct kelpo_polygon_triangle_s triangle;

    /* Default rendering options.*/
    const char *rendererName = "opengl_1_1";
    const unsigned renderDeviceIdx = 0;
    const unsigned windowWidth = 1920;
    const unsigned windowHeight = 1080;
    const unsigned windowBPP = 32;

    /* Initialize Kelpo.*/
    if (!kelpo_create_interface(&kelpo, rendererName) ||
        !kelpo->window.open(renderDeviceIdx, windowWidth, windowHeight, windowBPP) ||
        !kelpo->window.set_message_handler(window_message_handler))
    {
        fprintf(stderr, "Failed to initialize Kelpo.\n");
        goto cleanup;
    }

    /* Create the triangle that we'll render.*/
    {
        const int widthHalf = (windowWidth / 2);
        const int heightHalf = (windowHeight / 2);
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

    /* Render the triangle (until the user presses Esc).*/
    while (kelpo->window.process_messages(),
           !kelpo->window.is_closing())
    {
        kelpo->rasterizer.clear_frame();
        kelpo->rasterizer.draw_triangles(&triangle, 1);
        kelpo->window.flip_surface();

        if (kelpo_error_peek() != KELPOERR_ALL_GOOD)
        {
            fprintf(stderr, "Kelpo has reported an error.\n");
            goto cleanup;
        }
    }

    cleanup:

    if (!kelpo_release_interface(kelpo))
    {
        fprintf(stderr, "Failed to release Kelpo.\n");
    }

    return (kelpo_error_peek() == KELPOERR_ALL_GOOD)
           ? EXIT_SUCCESS
           : EXIT_FAILURE;
}
