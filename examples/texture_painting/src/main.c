/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Loads and renders using the Kelpo renderer a rotating cube whose texture's
 * pixels are modified on the fly.
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <kelpo_interface/generic_stack.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/interface.h>
#include <kelpo_interface/common/globals.h>
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

/* Generates mipmaps for the given texture of its base mip level down to 1 x 1.*/
static void regenerate_texture_mipmaps(struct kelpo_polygon_texture_s *const texture)
{
    unsigned m = 0;
    
    assert((texture->width == texture->height) &&
           "Expected the texture to be square.");

    assert(((unsigned)(log(texture->width) / log(2) + 1.5) == texture->numMipLevels) &&
           "Expected the texture to have mip levels down to 1 x 1.");

    for (m = 1; m < texture->numMipLevels; m++)
    {
        unsigned x = 0;
        unsigned y = 0;
        const unsigned mipLevelSideLength = (texture->width / pow(2, m));
        const unsigned minification = (texture->width / mipLevelSideLength);
        
        /* Simple nearest neighbor downscaling from the base mip level.*/
        for (y = 0; y < mipLevelSideLength; y++)
        {
            for (x = 0; x < mipLevelSideLength; x++)
            {
                const unsigned dstIdx = x + y * mipLevelSideLength;
                const unsigned srcIdx = (x * minification) + (y * minification) * texture->width;
                texture->mipLevel[m][dstIdx] = texture->mipLevel[0][srcIdx];
            }
        }
    }

    return;
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
    struct kelpo_interface_s renderer = kelpo_create_interface("opengl_1_2");
    
    uint32_t numTextures = 0;
    struct kelpo_polygon_texture_s *textures = NULL;
    struct kelpo_generic_stack_s *triangles = kelpo_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpo_generic_stack_s *transformedTriangles = kelpo_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));

    struct kelpo_polygon_texture_s *fontTexture = kelpo_text_mesh__create_font();

    /* Process any relevant command-line parameters.*/
    {
        int c = 0;
        while ((c = kelpo_cliparse(argc, argv)) != -1)
        {
            switch (c)
            {
                case 'r':
                {
                    renderer = kelpo_create_interface(kelpo_cliparse_optarg());
                    break;
                }
                case 'v':
                {
                    vsyncEnabled = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    break;
                }
                case 'w':
                {
                    renderResolution.width = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    assert((renderResolution.width != 0u) && "Invalid render width.");
                    break;
                }
                case 'h':
                {
                    renderResolution.height = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    assert((renderResolution.height != 0u) && "Invalid render height.");
                    break;
                }
                case 'b':
                {
                    renderResolution.bpp = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    assert((renderResolution.bpp != 0u) && "Invalid render bit depth.");
                    break;
                }
                case 'd':
                {
                    /* The device index is expected to be 1-indexed (device #1 is 1).*/
                    renderDeviceIdx = strtoul(kelpo_cliparse_optarg(), NULL, 10);
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

        sprintf(windowTitle, "Kelpo %d.%d.%d / %s (%d.%d.%d)",
                KELPO_INTERFACE_VERSION_MAJOR,
                KELPO_INTERFACE_VERSION_MINOR,
                KELPO_INTERFACE_VERSION_PATCH,
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

        if (!kelpo_load_kac10_mesh("cube.kac", triangles, &textures, &numTextures) ||
            !triangles->count)
        {
            fprintf(stderr, "ERROR: Could not load the cube model.\n");
            return 1;
        }

        assert(numTextures &&
               "Expected the cube model to only have one texture.");

        for (i = 0; i < numTextures; i++)
        {
            renderer.rasterizer.upload_texture(&textures[i]);
        }

        /* Triangle transformation may produce more triangles than there were
         * before transformation, due to frustum clipping etc. - but also fewer
         * due to back-face culling and so on.*/
        kelpo_generic_stack__grow(transformedTriangles, (triangles->capacity * 1.3));
    }

    /* Render.*/
    while (renderer.window.is_window_open())
    {
        renderer.window.process_events();

        kelpo_generic_stack__clear(transformedTriangles);
        trirot_transform_and_rotate_triangles(triangles,
                                              transformedTriangles,
                                              0, 0, 4.7,
                                              0.0035, 0.006, 0.0035);

        /* Print the UI text.*/
        {
            char fpsString[10];
            const unsigned fps = framerate();

            sprintf(fpsString, "FPS: %d", ((fps > 999)? 999 : fps));

            kelpo_text_mesh__print(renderer.metadata.rendererName, 255, 255, 255, 25, 30, transformedTriangles);
            kelpo_text_mesh__print(fpsString, 200, 200, 200, 25, 60, transformedTriangles);
        }

        /* Paint circles onto the cube's texture. We'll add one new pixel each
         * successive frame.*/
        {
            static unsigned angle = 0;
            static unsigned radius = 50;
            unsigned offset = (radius * 1.25);
            const unsigned x = (radius * cos(angle * (M_PI / 180)));
            const unsigned y = (radius * sin(angle * (M_PI / 180)));

            textures[0].mipLevel[0][(offset + x) + (offset + y) * textures[0].width] = 0xffff;
            
            regenerate_texture_mipmaps(&textures[0]);

            /* Ask the render API to re-download the texture's pixel data, so
             * the drawing we've done shows up in the rendered image.*/
            renderer.rasterizer.update_texture(&textures[0]);

            /* Start with a new circle every time we've finished drawing the
             * previous one.*/
            if ((angle += 4) >= 360)
            {
                angle = 0;
                radius -= (radius != 0);
            }
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

        kelpo_generic_stack__free(triangles);
        kelpo_generic_stack__free(transformedTriangles);
    }

    return 0;
}
