/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Loads and renders a simple rotating cube model using the shiet renderer.
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <shiet_interface/polygon/triangle/triangle_stack.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include <shiet_interface/interface.h>
#include <shiet_interface/common/globals.h>
#include "../../common_src/transform_and_rotate_triangles.h"
#include "../../common_src/parse_command_line.h"
#include "../../common_src/load_kac_1_0_mesh.h"

int main(int argc, char *argv[])
{
    struct { unsigned width; unsigned height; } renderResolution = {640, 480};
    struct shiet_interface_s renderer = shiet_create_interface("opengl_1_2");
    
    uint32_t numTextures = 0;
    struct shiet_polygon_texture_s *textures = NULL;
    struct shiet_polygon_triangle_stack_s *triangles = shiet_tristack_create(1);
    struct shiet_polygon_triangle_stack_s *transformedTriangles = shiet_tristack_create(1);

    /* Process any relevant command-line parameters.*/
    {
        int c = 0;
        while ((c = shiet_cliparse(argc, argv)) != -1)
        {
            switch (c)
            {
                case 'r':
                {
                    renderer = shiet_create_interface(shiet_cliparse_optarg());
                    break;
                }
                case 'w':
                {
                    renderResolution.width = strtoul(shiet_cliparse_optarg(), NULL, 10);
                    assert((renderResolution.width != 0u) && "Invalid render width.");
                    break;
                }
                case 'h':
                {
                    renderResolution.height = strtoul(shiet_cliparse_optarg(), NULL, 10);
                    assert((renderResolution.height != 0u) && "Invalid render height.");
                    break;
                }
                default: break;
            }
        }
    }

    /* Initialize the renderer.*/
    {
        char windowTitle[128];

        sprintf(windowTitle, "shiet %d.%d.%d / %s (%d.%d.%d)",
                SHIET_INTERFACE_VERSION_MAJOR,
                SHIET_INTERFACE_VERSION_MINOR,
                SHIET_INTERFACE_VERSION_PATCH,
                renderer.metadata.rendererName,
                renderer.metadata.rendererVersionMajor,
                renderer.metadata.rendererVersionMinor,
                renderer.metadata.rendererVersionPatch);

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
        uint32_t i = 0, m = 0;

        for (i = 0; i < numTextures; i++)
        {
            for (m = 0; m < textures[i].numMipLevels; m++)
            {
                free(textures[i].mipLevel[m]);
            }
        }
        free(textures);

        shiet_tristack_free(triangles);
        shiet_tristack_free(transformedTriangles);
    }

    return 0;
}
