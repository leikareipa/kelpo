/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Loads and renders a simple rotating cube model using the shiet renderer.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <shiet/polygon/triangle/triangle_stack.h>
#include <shiet/polygon/triangle/triangle.h>
#include <shiet/renderer_interface.h>
#include <shiet/common/globals.h>
#include "../../common_src/transform_and_rotate_triangles.h"
#include "../../common_src/load_kac_1_0_mesh.h"

int main(void)
{
    const struct { unsigned width; unsigned height; } renderResolution = {640, 480};
    struct shiet_renderer_interface_s renderer = shiet_create_renderer_interface("OpenGL");
    
    uint32_t numTextures = 0;
    struct shiet_polygon_texture_s *textures = NULL;
    struct shiet_polygon_triangle_stack_s *triangles = shiet_tristack_create(1);
    struct shiet_polygon_triangle_stack_s *transformedTriangles = shiet_tristack_create(1);

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

    /* Load in the cube model.*/
    {
        uint32_t i = 0;

        if (!shiet_load_kac10_mesh("cube.kac", triangles, &textures, &numTextures) ||
            !triangles->count)
        {
            fprintf(stderr, "ERROR: Could not load the cube model.\n");
            return 1;
        }

        for (i = 0; i < numTextures; i++)
        {
            renderer.rasterizer.upload_texture(&textures[i]);
        }

        shiet_tristack_grow(transformedTriangles, triangles->capacity);
    }

    /* Render.*/
    while (renderer.window.is_window_open())
    {
        shiet_tristack_clear(transformedTriangles);

        trirot_transform_and_rotate_triangles(triangles,
                                              transformedTriangles,
                                              0, 0, 4.7,
                                              0.0035, 0.006, 0.0035);

        renderer.rasterizer.clear_frame();
        renderer.rasterizer.draw_triangles(transformedTriangles->data, transformedTriangles->count);
        renderer.window.update_window();
    }

    /* Release memory.*/
    {
        uint32_t i = 0;

        for (i = 0; i < numTextures; i++)
        {
            free(textures[i].pixelArray);
            free(textures[i].pixelArray16bit);
        }

        free(textures);
        shiet_tristack_free(triangles);
        shiet_tristack_free(transformedTriangles);
    }

    return 0;
}
