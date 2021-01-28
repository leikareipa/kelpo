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
#include <kelpo_auxiliary/load_kac_1_0_mesh.h>
#include <kelpo_auxiliary/triangle_preparer.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_auxiliary/text_mesh.h>
#include <kelpo_auxiliary/matrix_44.h>
#include <kelpo_auxiliary/misc.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/interface.h>
#include <kelpo_interface/error.h>
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
static int regenerate_mipmaps_for_texture(struct kelpo_polygon_texture_s *const texture)
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

    return 1;
}

int main(int argc, char *argv[])
{
    const struct kelpo_interface_s *kelpo = NULL;

    uint32_t i = 0, numTextures = 0;
    struct kelpo_polygon_texture_s *textures = NULL;
    struct kelpo_polygon_texture_s *fontTexture = NULL;
    struct kelpoa_generic_stack_s *triangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *worldSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *screenSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));

    struct kelpoa_matrix44_s clipSpaceMatrix;
    struct kelpoa_matrix44_s screenSpaceMatrix;

    /* Set up default rendering options, and parse the command-line to see if
     * the user has provided any overrides for them.*/
    struct cliparse_params_s cliArgs = {0};
    cliArgs.rendererName = "opengl_1_2";
    cliArgs.windowWidth = 1920;
    cliArgs.windowHeight = 1080;
    cliArgs.windowBPP = 32;
    cliparse_get_params(argc, argv, &cliArgs);

    /* Initialize Kelpo.*/
    if (!kelpo_create_interface(&kelpo, cliArgs.rendererName) ||
        !kelpo->window.open(cliArgs.renderDeviceIdx, cliArgs.windowWidth, cliArgs.windowHeight, cliArgs.windowBPP) ||
        !kelpo->window.set_message_handler(default_window_message_handler))
    {
        fprintf(stderr, "Failed to initialize Kelpo.\n");
        goto cleanup;
    }

    /* Load assets from disk and send them to Kelpo.*/
    {
        uint32_t i = 0;

        /* The cube model.*/
        {
            if (!kelpoa_load_kac10_mesh("cube.kac", triangles, &textures, &numTextures) ||
                !triangles->count)
            {
                fprintf(stderr, "Failed to load the model's data.\n");
                goto cleanup;
            }

            /* Transfer the textures to Kelpo.*/
            for (i = 0; i < numTextures; i++)
            {
                kelpo->rasterizer.upload_texture(&textures[i]);
            }
        }

        /* The font.*/
        {
            fontTexture = kelpoa_text_mesh__create_font();
            kelpo->rasterizer.upload_texture(fontTexture);
            
            /* We can free the font texture's pixel data, since Kelpo has now
             * made a copy of it for itself (probably in VRAM).*/
            free(fontTexture->mipLevel[0]);
            fontTexture->mipLevel[0] = NULL;
        }

        /* A catch-all for Kelpo errors that may have occurred while we transferred
         * asset data to Kelpo.*/
        if (kelpo_error_peek() != KELPOERR_ALL_GOOD)
        {
            fprintf(stderr, "Failed to transfer asset data to Kelpo.\n");
            goto cleanup;
        }
    }

    /* Initialize transformation matrices, for rotating the triangle.*/
    {
        kelpoa_matrix44__make_clip_space_matrix(&clipSpaceMatrix,
                                                KELPOA_DEG_TO_RAD(60),
                                                (cliArgs.windowWidth / (float)cliArgs.windowHeight),
                                                0.1, 100);

        kelpoa_matrix44__make_screen_space_matrix(&screenSpaceMatrix,
                                                  (cliArgs.windowWidth / 2.0f),
                                                  (cliArgs.windowHeight / 2.0f));
    }

    /* Render loop.*/
    while (kelpo->window.process_messages(),
           !kelpo->window.is_closing())
    {
        static float rotX = 0, rotY = 0, rotZ = 0;

        /* Rotate the cube model and transform its vertices into screen space.*/
        kelpoa_generic_stack__clear(worldSpaceTriangles);
        kelpoa_generic_stack__clear(screenSpaceTriangles);
        kelpoa_triprepr__duplicate_triangles(triangles, worldSpaceTriangles);
        kelpoa_triprepr__rotate_triangles(worldSpaceTriangles, (rotX += 0.0035), (rotY += 0.006), (rotZ += 0.0035));
        kelpoa_triprepr__translate_triangles(worldSpaceTriangles, 0, 0, 4.7);
        kelpoa_triprepr__project_triangles_to_screen(worldSpaceTriangles,
                                                     screenSpaceTriangles,
                                                     &clipSpaceMatrix,
                                                     &screenSpaceMatrix,
                                                     0.1, 100, 1);

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

            kelpoa_text_mesh__print(screenSpaceTriangles, kelpo->metadata.rendererName, 25, 30, 255, 255, 255, 1);
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
            
            /* Ask the render API to re-download the texture's pixel data, so
             * the drawing we've done shows up in the rendered image.*/
            if (!regenerate_mipmaps_for_texture(&textures[0]) ||
                !kelpo->rasterizer.update_texture(&textures[0]))
            {
                fprintf(stderr, "Failed to update Kelpo's texture data.\n");
                goto cleanup;
            }

            /* Start with a new circle every time we've finished drawing the
             * previous one.*/
            if ((angle += 4) >= 360)
            {
                angle = 0;
                radius -= (radius != 0);
            }
        }

        /* Render the cube.*/
        kelpo->rasterizer.clear_frame();
        kelpo->rasterizer.draw_triangles(screenSpaceTriangles->data, screenSpaceTriangles->count);
        kelpo->window.flip_surface();

        if (kelpo_error_peek() != KELPOERR_ALL_GOOD)
        {
            fprintf(stderr, "Kelpo has reported an error.\n");
            goto cleanup;
        }
    }

    cleanup:

    for (i = 0; i < numTextures; i++)
    {
        uint32_t m = 0;

        for (m = 0; m < textures[i].numMipLevels; m++)
        {
            free(textures[i].mipLevel[m]);
            textures[i].mipLevel[m] = NULL;
        }
    }

    free(textures);
    free(fontTexture);
    kelpoa_generic_stack__free(triangles);
    kelpoa_generic_stack__free(worldSpaceTriangles);
    kelpoa_generic_stack__free(screenSpaceTriangles);

    if (!kelpo_release_interface(kelpo))
    {
        fprintf(stderr, "Failed to release Kelpo.\n");
    }

    return (kelpo_error_peek() == KELPOERR_ALL_GOOD)
           ? EXIT_SUCCESS
           : EXIT_FAILURE;
}
