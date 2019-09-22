#include <stdio.h>
#include "shiet/polygon/triangle/triangle.h"
#include "shiet/common/globals.h"
#include "shiet/interface.h"

int main(void)
{
    char windowTitle[64];
    struct shiet_polygon_triangle_s triangles[1];
    struct shiet_render_interface_s shiet = shiet_create_render_interface("opengl");

    snprintf(windowTitle, NUM_ARRAY_ELEMENTS(windowTitle),
             "shiet %d.%d.%d", shiet.metadata.shietMajorVersion,
                               shiet.metadata.shietMinorVersion,
                               shiet.metadata.shietPatchVersion);

    shiet.initialize(800, 600, windowTitle);

    triangles[0].vertex[0].x = 250;
    triangles[0].vertex[0].y = 150;
    triangles[0].vertex[1].x = 550;
    triangles[0].vertex[1].y = 150;
    triangles[0].vertex[2].x = 550;
    triangles[0].vertex[2].y = 450;
    triangles[0].material.texture = NULL;
    triangles[0].material.baseColor[0] = 128;
    triangles[0].material.baseColor[1] = 192;
    triangles[0].material.baseColor[2] = 64;

    while (shiet.window.is_window_open())
    {
        shiet.rasterizer.clear_frame();
        shiet.rasterizer.draw_triangles(triangles, 1);
        shiet.window.update_window();
    }

    return 0;
}
