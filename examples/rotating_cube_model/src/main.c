/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Loads and renders a simple rotating cube model using the shiet renderer.
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
#include "../../common_src/load_kac_1_0_mesh.h"

int main(void)
{
    const struct { unsigned width; unsigned height; } renderResolution = {1024, 768};
    
    uint32_t numTriangles = 0, numTextures = 0;
    struct shiet_polygon_texture_s *textures = NULL;
    struct shiet_polygon_triangle_s *triangles = NULL;
    struct shiet_polygon_triangle_s *transformedTriangles = NULL;
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

    /* Load the cube model from a KAC 1.0 mesh file.*/
    {
        uint32_t i = 0;

        if (!shiet_load_kac10_mesh("cube.kac", &triangles, &numTriangles, &textures, &numTextures))
        {
            fprintf(stderr, "ERROR: Could not load the cube model.\n");
            return 1;
        }

        for (i = 0; i < numTextures; i++)
        {
            renderer.rasterizer.upload_texture(&textures[i]);
        }

        transformedTriangles = malloc(sizeof(struct shiet_polygon_triangle_s) * numTriangles);
    }

    /* Render.*/
    while (renderer.window.is_window_open())
    {
        const uint32_t sceneTriangleCount = trirot_transform_and_rotate_triangles(triangles, numTriangles, transformedTriangles,
                                                                                  0.0035, 0.006, 0.0035,
                                                                                  4.7);

        renderer.rasterizer.clear_frame();
        renderer.rasterizer.draw_triangles(transformedTriangles, sceneTriangleCount);
        renderer.window.update_window();
    }

    return 0;
}
