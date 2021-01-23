/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Renders a simple static triangle using the Kelpo renderer.
 * 
 */

#include <windows.h>
#include <kelpo_interface/interface.h>
#include <kelpo_interface/polygon/triangle/triangle.h>

LRESULT window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    /* Exit the program on ESC.*/
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
    const char *rendererName = "opengl_1_2";
    const unsigned renderDeviceIdx = 0;
    const unsigned windowWidth = 1920;
    const unsigned windowHeight = 1080;
    const unsigned windowBPP = 32;

    /* Initialize the renderer.*/
    {
        kelpo = kelpo_create_interface(rendererName);

        kelpo->window.open(renderDeviceIdx,
                           windowWidth,
                           windowHeight,
                           windowBPP);

        kelpo->window.set_message_handler(window_message_handler);
    }

    /* Create the triangle that will be rendered in the middle of the screen.*/
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

    /* Render the triangle.*/
    while (kelpo->window.is_open())
    {
        kelpo->window.process_messages();

        kelpo->rasterizer.clear_frame();
        kelpo->rasterizer.draw_triangles(&triangle, 1);

        kelpo->window.flip_surface();
    }

    kelpo_release_interface(kelpo);

    return 0;
}
