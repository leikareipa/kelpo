/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Renders a simple rotating triangle using the shiet renderer.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <shiet/polygon/triangle/triangle_stack.h>
#include <shiet/polygon/triangle/triangle.h>
#include <shiet/renderer_interface.h>
#include <shiet/common/globals.h>
#include "../../common_src/transform_and_rotate_triangles.h"

int main(void)
{
    const struct { unsigned width; unsigned height; } renderResolution = {640, 480};
    struct shiet_renderer_interface_s renderer = shiet_create_renderer_interface("OpenGL");

    struct shiet_polygon_triangle_stack_s *triangles = shiet_tristack_create(1);
    struct shiet_polygon_triangle_stack_s *transformedTriangles = shiet_tristack_create(triangles->capacity);

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

    /* Create the triangle that will be rendered in the middle of the screen.*/
    {
        struct shiet_polygon_triangle_s triangle;

        triangle.vertex[0].x = -0.5;
        triangle.vertex[0].y = -0.5;
        triangle.vertex[0].z = 0;
        triangle.vertex[0].w = 1;
        triangle.vertex[0].u = 0;
        triangle.vertex[0].v = 0;

        triangle.vertex[1].x = 0.5;
        triangle.vertex[1].y = -0.5;
        triangle.vertex[1].z = 0;
        triangle.vertex[1].w = 1;
        triangle.vertex[1].u = 1;
        triangle.vertex[1].v = 0;

        triangle.vertex[2].x = 0.5;
        triangle.vertex[2].y = 0.5;
        triangle.vertex[2].z = 0;
        triangle.vertex[2].w = 1;
        triangle.vertex[2].u = 1;
        triangle.vertex[2].v = 1;
        
        triangle.material.texture = NULL;
        triangle.material.baseColor[0] = 128;
        triangle.material.baseColor[1] = 192;
        triangle.material.baseColor[2] = 64;

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
