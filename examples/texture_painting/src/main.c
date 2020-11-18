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
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_auxiliary/load_kac_1_0_mesh.h>
#include <kelpo_auxiliary/text_mesh.h>
#include <kelpo_auxiliary/triangle_preparer.h>
#include <kelpo_auxiliary/matrix_44.h>
#include <kelpo_auxiliary/misc.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/interface.h>
#include "../../common_src/default_window_message_handler.h"
#include "../../common_src/parse_command_line.h"

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
    /* The location, in world units, of the near and far clipping planes.*/
    const float zNear = 0.1;
    const float zFar = 100;
    
    uint32_t numTextures = 0;
    struct kelpo_polygon_texture_s *textures = NULL;
    struct kelpoa_generic_stack_s *triangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *worldSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *screenSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpo_polygon_texture_s *fontTexture = kelpoa_text_mesh__create_font();

    struct kelpoa_matrix44_s clipSpaceMatrix;
    struct kelpoa_matrix44_s screenSpaceMatrix;

    struct kelpo_interface_s renderer;

    /* Set up default rendering options, and parse the command-line to see if
     * the user has provided any overrides for them.*/
    struct cliparse_params_s cliParams = {0};
    {
        cliParams.rendererName = "opengl_1_2";
        cliParams.windowWidth = 1920;
        cliParams.windowHeight = 1080;
        cliParams.windowBPP = 32;

        cliparse_get_params(argc, argv, &cliParams);
    }

    /* Initialize the renderer.*/
    {
        renderer = kelpo_create_interface(cliParams.rendererName);

        renderer.window.open(cliParams.renderDeviceIdx,
                             cliParams.windowWidth,
                             cliParams.windowHeight,
                             cliParams.windowBPP);
                            
        renderer.window.set_message_handler(default_window_message_handler);

        renderer.rasterizer.upload_texture(fontTexture);
        free(fontTexture->mipLevel[0]);
        fontTexture->mipLevel[0] = NULL;
    }

    /* Load the cube model.*/
    {
        uint32_t i = 0;

        if (!kelpoa_load_kac10_mesh("cube.kac", triangles, &textures, &numTextures) ||
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
    }

    kelpoa_matrix44__make_clip_space_matrix(&clipSpaceMatrix,
                                            KELPOA_DEG_TO_RAD(60),
                                            (cliParams.windowWidth / (float)cliParams.windowHeight),
                                            zNear,
                                            zFar);

    kelpoa_matrix44__make_screen_space_matrix(&screenSpaceMatrix,
                                              (cliParams.windowWidth / 2.0f),
                                              (cliParams.windowHeight / 2.0f));


    /* Render.*/
    while (renderer.window.is_open())
    {
        static float rotX = 0, rotY = 0, rotZ = 0;
        rotX += 0.0035;
        rotY += 0.006;
        rotZ += 0.0035;

        renderer.window.process_messages();

        /* Transform the scene's triangles into screen space.*/
        kelpoa_generic_stack__clear(worldSpaceTriangles);
        kelpoa_generic_stack__clear(screenSpaceTriangles);
        kelpoa_triprepr__duplicate_triangles(triangles, worldSpaceTriangles);
        kelpoa_triprepr__rotate_triangles(worldSpaceTriangles, rotX, rotY, rotZ);
        kelpoa_triprepr__translate_triangles(worldSpaceTriangles, 0, 0, 4.7);
        kelpoa_triprepr__project_triangles_to_screen(worldSpaceTriangles,
                                                     screenSpaceTriangles,
                                                     &clipSpaceMatrix,
                                                     &screenSpaceMatrix,
                                                     zNear,
                                                     zFar,
                                                     1);

        /* Print the UI text.*/
        {
            char fpsString[10];
            char polyString[50];
            const unsigned numScreenPolys = screenSpaceTriangles->count;
            const unsigned numWorldPolys = worldSpaceTriangles->count;
            const unsigned fps = framerate();

            sprintf(fpsString, "FPS: %d", ((fps > 999)? 999 : fps));
            sprintf(polyString, "Polygons: %d/%d", ((numScreenPolys > 9999999)? 9999999 : numScreenPolys),
                                                   ((numWorldPolys > 9999999)? 9999999 : numWorldPolys));

            kelpoa_text_mesh__print(screenSpaceTriangles, renderer.metadata.rendererName, 25, 30, 255, 255, 255, 1);
            kelpoa_text_mesh__print(screenSpaceTriangles, polyString, 25, 60, 200, 200, 200, 1);
            kelpoa_text_mesh__print(screenSpaceTriangles, fpsString, 25, 90, 200, 200, 200, 1);
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
        renderer.rasterizer.draw_triangles(screenSpaceTriangles->data,
                                           screenSpaceTriangles->count);

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

        kelpoa_generic_stack__free(triangles);
        kelpoa_generic_stack__free(worldSpaceTriangles);
        kelpoa_generic_stack__free(screenSpaceTriangles);
    }

    return 0;
}
