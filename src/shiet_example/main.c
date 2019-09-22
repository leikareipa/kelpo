#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "shiet/polygon/triangle/triangle.h"
#include "shiet/common/globals.h"
#include "shiet/interface.h"

int main(void)
{
    char windowTitle[64];
    struct shiet_polygon_texture_s texture;
    struct shiet_polygon_triangle_s triangles[1];
    struct shiet_render_interface_s shiet = shiet_create_render_interface("opengl");

    snprintf(windowTitle, NUM_ARRAY_ELEMENTS(windowTitle),
             "shiet %d.%d.%d", shiet.metadata.shietMajorVersion,
                               shiet.metadata.shietMinorVersion,
                               shiet.metadata.shietPatchVersion);

    shiet.initialize(800, 600, windowTitle);

    /* Set up a basic texture, for testing.*/
    {
        texture.width = 2;
        texture.height = 2;
        texture.pixelArray = malloc(texture.width * texture.height * 4);
        memset(texture.pixelArray, 255, (texture.width * texture.height * 4));

        /* Top left.*/
        texture.pixelArray[0] = 255;
        texture.pixelArray[1] = 0;
        texture.pixelArray[2] = 0;

        /* Top right.*/
        texture.pixelArray[4] = 0;
        texture.pixelArray[5] = 255;
        texture.pixelArray[6] = 0;

        /* Bottom left.*/
        texture.pixelArray[8] = 255;
        texture.pixelArray[9] = 0;
        texture.pixelArray[10] = 255;

        /* Bottom right.*/
        texture.pixelArray[12] = 0;
        texture.pixelArray[13] = 0;
        texture.pixelArray[14] = 255;

        shiet.rasterizer.upload_texture(&texture);
    }

    /* Set up a basic triangle in the middle of the screen, for testing.*/
    {
        triangles[0].vertex[0].x = 250;
        triangles[0].vertex[0].y = 150;
        triangles[0].vertex[0].w = 1;
        triangles[0].vertex[0].u = 0;
        triangles[0].vertex[0].v = 0;

        triangles[0].vertex[1].x = 550;
        triangles[0].vertex[1].y = 150;
        triangles[0].vertex[1].w = 1;
        triangles[0].vertex[1].u = 1;
        triangles[0].vertex[1].v = 0;

        triangles[0].vertex[2].x = 550;
        triangles[0].vertex[2].y = 450;
        triangles[0].vertex[2].w = 1;
        triangles[0].vertex[2].u = 1;
        triangles[0].vertex[2].v = 1;
        
        triangles[0].material.texture = &texture;
        triangles[0].material.baseColor[0] = 128;
        triangles[0].material.baseColor[1] = 192;
        triangles[0].material.baseColor[2] = 64;
    }

    while (shiet.window.is_window_open())
    {
        shiet.rasterizer.clear_frame();
        shiet.rasterizer.draw_triangles(triangles, 1);
        shiet.window.update_window();
    }

    free(texture.pixelArray);
    return 0;
}
