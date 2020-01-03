/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Renders a simple rotating triangle using the shiet renderer.
 * 
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <shiet/polygon/triangle/triangle.h>
#include <shiet/renderer_interface.h>
#include <shiet/common/globals.h>
#include "../../common_src/transform_and_rotate_triangles.h"

int main(void)
{
    time_t timer = time(NULL); /* Used to toggle between linear and nearest texture filtering.*/
    const struct { unsigned width; unsigned height; } renderResolution = {1024, 768};

    struct shiet_polygon_texture_s texture;
    struct shiet_polygon_triangle_s triangles[1];
    struct shiet_polygon_triangle_s transformedTriangles[1];
    struct shiet_renderer_interface_s renderer = shiet_create_renderer_interface("OpenGL");

    /* Initialize the renderer.*/
    {
        char windowTitle[128];

        sprintf(windowTitle, "shiet %d.%d.%d / %s renderer %d.%d.%d",
                renderer.metadata.shietMajorVersion,
                renderer.metadata.shietMinorVersion,
                renderer.metadata.shietPatchVersion,
                renderer.metadata.rendererName,
                renderer.metadata.rendererMajorVersion,
                renderer.metadata.rendererMinorVersion,
                renderer.metadata.rendererPatchVersion);

        renderer.initialize(renderResolution.width, renderResolution.height, windowTitle);

        trirot_initialize_screen_geometry(renderResolution.width, renderResolution.height);
    }

    /* Set up a basic texture.*/
    {
        texture.width = 2;
        texture.height = 2;
        texture.filtering = SHIET_TEXTURE_FILTER_LINEAR;
        texture.pixelArray = malloc(texture.width * texture.height * 4);
        texture.pixelArray16bit = malloc(texture.width * texture.height * sizeof(texture.pixelArray16bit[0]));

        /* TODO: Set the texture's wrapping mode to clamp-to-edge.*/

        /* Top left.*/
        texture.pixelArray[0] = 255;
        texture.pixelArray[1] = 0;
        texture.pixelArray[2] = 0;
        texture.pixelArray[3] = 255;

        /* Top right.*/
        texture.pixelArray[4] = 0;
        texture.pixelArray[5] = 255;
        texture.pixelArray[6] = 0;
        texture.pixelArray[7] = 255;

        /* Bottom left.*/
        texture.pixelArray[8] = 255;
        texture.pixelArray[9] = 0;
        texture.pixelArray[10] = 255;
        texture.pixelArray[11] = 255;

        /* Bottom right.*/
        texture.pixelArray[12] = 0;
        texture.pixelArray[13] = 0;
        texture.pixelArray[14] = 255;
        texture.pixelArray[15] = 255;

        texture.pixelArray16bit[0] = 0xf801; /* Top left:  255,0,0.  */
        texture.pixelArray16bit[1] = 0x7c1;  /* Top right: 0,255,0.  */
        texture.pixelArray16bit[2] = 0xf83f; /* Top right: 255,0,255.*/
        texture.pixelArray16bit[3] = 0x3f;   /* Top right: 0,0,255.  */

        renderer.rasterizer.upload_texture(&texture);
    }

    /* Set up a triangle in the middle of the screen.*/
    {
        triangles[0].vertex[0].x = -0.5;
        triangles[0].vertex[0].y = -0.5;
        triangles[0].vertex[0].z = 0;
        triangles[0].vertex[0].w = 1;
        triangles[0].vertex[0].u = 0;
        triangles[0].vertex[0].v = 0;

        triangles[0].vertex[1].x = 0.5;
        triangles[0].vertex[1].y = -0.5;
        triangles[0].vertex[1].z = 0;
        triangles[0].vertex[1].w = 1;
        triangles[0].vertex[1].u = 1;
        triangles[0].vertex[1].v = 0;

        triangles[0].vertex[2].x = 0.5;
        triangles[0].vertex[2].y = 0.5;
        triangles[0].vertex[2].z = 0;
        triangles[0].vertex[2].w = 1;
        triangles[0].vertex[2].u = 1;
        triangles[0].vertex[2].v = 1;
        
        triangles[0].material.texture = &texture;
        triangles[0].material.baseColor[0] = 128;
        triangles[0].material.baseColor[1] = 192;
        triangles[0].material.baseColor[2] = 64;
    }

    /* Render.*/
    while (renderer.window.is_window_open())
    {
        const uint32_t sceneTriangleCount = trirot_transform_and_rotate_triangles(triangles, 1, transformedTriangles,
                                                                                  0, 0, 1.5,
                                                                                  0, 0.01, 0);

        if ((time(NULL) - timer) > 3)
        {
            texture.filtering = ((texture.filtering == SHIET_TEXTURE_FILTER_NEAREST)?
                                 SHIET_TEXTURE_FILTER_LINEAR : SHIET_TEXTURE_FILTER_NEAREST);

            renderer.rasterizer.update_texture(&texture);

            timer = time(NULL);
        }

        renderer.rasterizer.clear_frame();
        renderer.rasterizer.draw_triangles(transformedTriangles, sceneTriangleCount);
        renderer.window.update_window();
    }

    free(texture.pixelArray);
    return 0;
}
