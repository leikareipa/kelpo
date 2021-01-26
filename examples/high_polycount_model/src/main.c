/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Renders a high-polycount 3d model using the Kelpo renderer.
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_auxiliary/load_kac_1_0_mesh.h>
#include <kelpo_auxiliary/triangle_preparer.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_auxiliary/matrix_44.h>
#include <kelpo_auxiliary/text_mesh.h>
#include <kelpo_auxiliary/misc.h>
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
        ((time(NULL) - frameRateTimer) >= 2))
    {
        framesPerSecond = (numFramesCounted / (time(NULL) - frameRateTimer));
        frameRateTimer = time(NULL);
        numFramesCounted = 0;
    }

    return framesPerSecond;
}

int main(int argc, char *argv[])
{
    const struct kelpo_interface_s *kelpo = NULL;

    uint32_t numTextures = 0;
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

    /* Load assets from disk and send them to Kelpo. Note that once we've uploaded
     * textures to Kelpo, we can free their local pixel (mipmap) data, since Kelpo
     * makes copies of those data (typically in VRAM). The texture object itself
     * should not be freed or modified until rendering is finished, however.*/
    {
        uint32_t i = 0, m = 0;

        /* The apricot model.*/
        {
            if (!kelpoa_load_kac10_mesh("apricot.kac", triangles, &textures, &numTextures) ||
                !triangles->count)
            {
                fprintf(stderr, "Failed to load the model's data.\n");
                goto cleanup;
            }

            /* Transfer the textures to Kelpo.*/
            for (i = 0; i < numTextures; i++)
            {
                kelpo->rasterizer.upload_texture(&textures[i]);

                for (m = 0; m < textures[i].numMipLevels; m++)
                {
                    free(textures[i].mipLevel[m]);
                    textures[i].mipLevel[m] = NULL;
                }
            }
        }

        /* The font.*/
        {
            fontTexture = kelpoa_text_mesh__create_font();
            kelpo->rasterizer.upload_texture(fontTexture);
            
            /* Note: We assume that the font only has one mip level.*/
            free(fontTexture->mipLevel[0]);
            fontTexture->mipLevel[0] = NULL;
        }

        /* A catch-all for Kelpo errors that may have occurred while we transferred
         * asset data to Kelpo.*/
        if (kelpo_error_peek() != KELPOERR_NO_ERROR)
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
           kelpo->window.is_open())
    {
        static float rotY = 0;

        /* Rotate the model and transform its vertices into screen space.*/
        kelpoa_generic_stack__clear(worldSpaceTriangles);
        kelpoa_generic_stack__clear(screenSpaceTriangles);
        kelpoa_triprepr__duplicate_triangles(triangles, worldSpaceTriangles);
        kelpoa_triprepr__rotate_triangles(worldSpaceTriangles, 0, (rotY += 0.01), 0);
        kelpoa_triprepr__translate_triangles(worldSpaceTriangles, 0, -2.5, 8.5);
        kelpoa_triprepr__project_triangles_to_screen(worldSpaceTriangles,
                                                     screenSpaceTriangles,
                                                     &clipSpaceMatrix,
                                                     &screenSpaceMatrix,
                                                     0.1, 100, 1);

        /* Print the UI text.*/
        {
            char fpsString[10];
            char polyString[50];
            const unsigned fps = framerate();
            const unsigned numScreenPolys = screenSpaceTriangles->count;
            const unsigned numWorldPolys = worldSpaceTriangles->count;

            sprintf(fpsString, "FPS: %d", ((fps > 999)? 999 : fps));
            sprintf(polyString, "Polygons: %d/%d", ((numScreenPolys > 9999999)? 9999999 : numScreenPolys),
                                                   ((numWorldPolys > 9999999)? 9999999 : numWorldPolys));

            kelpoa_text_mesh__print(screenSpaceTriangles, kelpo->metadata.rendererName, 25, 30, 255, 255, 255, 1);
            kelpoa_text_mesh__print(screenSpaceTriangles, polyString, 25, 60, 200, 200, 200, 1);
            kelpoa_text_mesh__print(screenSpaceTriangles, fpsString, 25, 90, 200, 200, 200, 1);
        }

        /* Render the model.*/
        kelpo->rasterizer.clear_frame();
        kelpo->rasterizer.draw_triangles(screenSpaceTriangles->data, screenSpaceTriangles->count);
        kelpo->window.flip_surface();

        if (kelpo_error_peek() != KELPOERR_NO_ERROR)
        {
            fprintf(stderr, "Kelpo has reported an error.\n");
            goto cleanup;
        }
    }

    cleanup:

    free(textures);
    free(fontTexture);
    kelpoa_generic_stack__free(triangles);
    kelpoa_generic_stack__free(worldSpaceTriangles);
    kelpoa_generic_stack__free(screenSpaceTriangles);

    if (!kelpo_release_interface(kelpo))
    {
        fprintf(stderr, "Failed to release Kelpo.\n");
    }

    return (kelpo_error_peek() != KELPOERR_NO_ERROR)
           ? EXIT_SUCCESS
           : EXIT_FAILURE;
}
