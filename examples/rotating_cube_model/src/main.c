/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Loads and renders a simple rotating cube model using the shiet renderer.
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <shiet_interface/generic_data_stack.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include <shiet_interface/interface.h>
#include <shiet_interface/common/globals.h>
#include "../../common_src/transform_and_rotate_triangles.h"
#include "../../common_src/parse_command_line.h"
#include "../../common_src/load_kac_1_0_mesh.h"
#include "../../common_src/text_mesh.h"

#include <windows.h>

/* Call this function once per frame and it'll tell you an estimate of the frame
 * rate (FPS).*/
static unsigned framerate(void)
{
    static unsigned numFramesCounted = 0;
    static unsigned framesPerSecond = 0;
    static unsigned frameRateTimer = 0;

    numFramesCounted++;

    if (!frameRateTimer ||
        (time(NULL) - frameRateTimer) >= 2)
    {
        framesPerSecond = (numFramesCounted / (time(NULL) - frameRateTimer));
        frameRateTimer = time(NULL);
        numFramesCounted = 0;
    }

    return framesPerSecond;
}

int main(int argc, char *argv[])
{
    /* An index in an enumeration of API-compatible devices on the system,
     * identifying the devide to be used in rendering.*/
    unsigned renderDeviceIdx = 0;

    /* If set to 1, we'll request the renderer to use vsync. Otherwise, we'll
     * ask for vsync to be off. On some hardware, this option will have no
     * effect, however.*/
    unsigned vsyncEnabled = 1;

    struct { unsigned width; unsigned height; unsigned bpp; } renderResolution = {640, 480, 16};
    struct shiet_interface_s renderer = shiet_create_interface("opengl_1_2");
    
    uint32_t numTextures = 0;
    struct shiet_polygon_texture_s *textures = NULL;
    struct shiet_generic_data_stack_s *triangles = shiet_generic_data_stack__create(1, sizeof(struct shiet_polygon_triangle_s));
    struct shiet_generic_data_stack_s *transformedTriangles = shiet_generic_data_stack__create(1, sizeof(struct shiet_polygon_triangle_s));

    struct shiet_polygon_texture_s *fontTexture = shiet_text_mesh__create_font();

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
                case 'v':
                {
                    vsyncEnabled = strtoul(shiet_cliparse_optarg(), NULL, 10);
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
                case 'b':
                {
                    renderResolution.bpp = strtoul(shiet_cliparse_optarg(), NULL, 10);
                    assert((renderResolution.bpp != 0u) && "Invalid render bit depth.");
                    break;
                }
                case 'd':
                {
                    /* The device index is expected to be 1-indexed (device #1 is 1).*/
                    renderDeviceIdx = strtoul(shiet_cliparse_optarg(), NULL, 10);
                    assert((renderDeviceIdx != 0u) && "Invalid render device index.");
                    renderDeviceIdx--;
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

        renderer.initialize(renderResolution.width,
                            renderResolution.height,
                            renderResolution.bpp,
                            vsyncEnabled,
                            renderDeviceIdx);
                            
        SetWindowTextA((HWND)renderer.window.get_handle(), windowTitle);

        trirot_initialize_screen_geometry(renderResolution.width, renderResolution.height);

        renderer.rasterizer.upload_texture(fontTexture);
        free(fontTexture->mipLevel[0]);
        fontTexture->mipLevel[0] = NULL;
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

        /* Triangle transformation may produce more triangles than there were
         * before transformation, due to frustum clipping etc. - but also fewer
         * due to back-face culling and so on.*/
        shiet_generic_data_stack__grow(transformedTriangles, (triangles->capacity * 1.3));
    }

    /* Render.*/
    while (renderer.window.is_window_open())
    {
        renderer.window.process_events();

        shiet_generic_data_stack__clear(transformedTriangles);
        trirot_transform_and_rotate_triangles(triangles,
                                              transformedTriangles,
                                              0, 0, 4.7,
                                              0.0035, 0.006, 0.0035);

        /* Print the UI text.*/
        {
            char fpsString[10];
            const unsigned fps = framerate();

            sprintf(fpsString, "FPS: %d", ((fps > 999)? 999 : fps));

            shiet_text_mesh__print(renderer.metadata.rendererName, 25, 30, transformedTriangles);
            shiet_text_mesh__print(fpsString, 25, 60, transformedTriangles);
        }

        renderer.rasterizer.clear_frame();
        renderer.rasterizer.draw_triangles(transformedTriangles->data,
                                           transformedTriangles->count);

        renderer.window.flip_surface();
    }

    /* Release any leftover memory.*/
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

        shiet_generic_data_stack__free(triangles);
        shiet_generic_data_stack__free(transformedTriangles);
    }

    return 0;
}
