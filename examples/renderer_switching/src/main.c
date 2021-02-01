/*
 * 2021 Tarpeeksi Hyvae Soft
 * 
 * LDemonstrates switching between Kelpo renderers on the fly.
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

#include <windows.h>

/* Set to 1 if the user has requested the renderer to be changed. Will be reset
 * to 0 when the change has been affected.*/
static int USER_WANTS_RENDERER_CHANGE = 0;

/* If the user has requested the renderer to be changed, this will hold the
 * name of the desired renderer.*/
static const char *NEXT_RENDERER_NAME = NULL;

/* If something goes wrong while attempting to set a new renderer, this will
 * hold the most recent error code related to that failure.*/
static enum kelpo_error_code_e RENDERER_CHANGE_ERROR_CODE = KELPOERR_ALL_GOOD;

/* If we fail to set a particular renderer, its name will be recorded here.*/
static const char *FAILED_RENDERER_NAME;

uint32_t NUM_TEXTURES = 0;
struct kelpo_polygon_texture_s *TEXTURES = NULL;
struct kelpo_polygon_texture_s *FONT_TEXTURE = NULL;

static LRESULT window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

/* Unloads the current Kelpo renderer and loads the next one, by the given name.
 * Returns 1 on success, 0 otherwise.*/
static int use_another_renderer(const struct kelpo_interface_s *kelpo,
                                const char *const rendererName,
                                const struct cliparse_params_s *const cliArgs)
{
    /* The previous renderer needs to be released.*/
    if (!kelpo_release_interface(kelpo))
    {
        fprintf(stderr, "Failed to release the old renderer.\n");
        return 0;
    }

    /* Initialize the new renderer.*/
    if (!kelpo_create_interface(&kelpo, rendererName) ||
        !kelpo->window.open(cliArgs->renderDeviceIdx,
                            cliArgs->windowWidth,
                            cliArgs->windowHeight,
                            cliArgs->windowBPP) ||
        !kelpo->window.set_message_handler(window_message_handler))
    {
        fprintf(stderr, "Failed to initialize the new renderer.\n");
        return 0;
    }

    /* Re-initializing the renderer causes Kelpo to release all its texture data,
     * so we need to re-upload it.*/
    {
        uint32_t i = 0;

        for (i = 0; i < NUM_TEXTURES; i++)
        {
            if (!kelpo->rasterizer.upload_texture(&TEXTURES[i]))
            {
                goto failed_texture_upload;
            }
        }

        if (!kelpo->rasterizer.upload_texture(FONT_TEXTURE))
        {
            goto failed_texture_upload;
        }
    }

    printf("Now using %s.\n", kelpo->metadata.rendererName);
    return 1;

    failed_texture_upload:
    fprintf(stderr, "Failed to upload textures.\n");
    return 0;
}

/* Polls for user input to find when the user is pressing on keys that indicate
 * they want to change the renderer.*/
static LRESULT window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    default_window_message_handler(windowHandle, message, wParam, lParam);

    switch (message)
    {
        case WM_KEYDOWN:
        {
            const int isRepeat = (lParam & 0x40000000);

            if (!isRepeat &&
                !USER_WANTS_RENDERER_CHANGE)
            {
                switch (wParam)
                {
                    case 0x31: case VK_NUMPAD1: NEXT_RENDERER_NAME = "opengl_1_1"; USER_WANTS_RENDERER_CHANGE = 1; break;
                    case 0x32: case VK_NUMPAD2: NEXT_RENDERER_NAME = "opengl_3_0"; USER_WANTS_RENDERER_CHANGE = 1; break;
                    case 0x33: case VK_NUMPAD3: NEXT_RENDERER_NAME = "direct3d_5"; USER_WANTS_RENDERER_CHANGE = 1; break;
                    case 0x34: case VK_NUMPAD4: NEXT_RENDERER_NAME = "direct3d_7"; USER_WANTS_RENDERER_CHANGE = 1; break;
                    case 0x35: case VK_NUMPAD5: NEXT_RENDERER_NAME = "glide_3"; USER_WANTS_RENDERER_CHANGE = 1; break;
                    default: break;
                }
            }

            break;
        }

        default: break;
    }

	return 0;
}

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
    const struct kelpo_interface_s *kelpo = NULL;

    struct kelpoa_generic_stack_s *triangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *worldSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *screenSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));

    struct kelpoa_matrix44_s clipSpaceMatrix;
    struct kelpoa_matrix44_s screenSpaceMatrix;

    /* Set up default rendering options, and parse the command-line to see if
     * the user has provided any overrides for them.*/
    struct cliparse_params_s cliArgs = {0};
    cliArgs.rendererName = "opengl_1_1";
    cliArgs.windowWidth = 1920;
    cliArgs.windowHeight = 1080;
    cliArgs.windowBPP = 32;
    cliparse_get_params(argc, argv, &cliArgs);

    NEXT_RENDERER_NAME = cliArgs.rendererName;
        
    /* Initialize Kelpo.*/
    if (!kelpo_create_interface(&kelpo, cliArgs.rendererName) ||
        !kelpo->window.open(cliArgs.renderDeviceIdx, cliArgs.windowWidth, cliArgs.windowHeight, cliArgs.windowBPP) ||
        !kelpo->window.set_message_handler(window_message_handler))
    {
        fprintf(stderr, "Failed to initialize Kelpo.\n");
        goto cleanup;
    }

    /* Load assets from disk and send them to Kelpo.*/
    {
        /* The cube model.*/
        {
            uint32_t i = 0;

            if (!kelpoa_load_kac10_mesh("cube.kac", triangles, &TEXTURES, &NUM_TEXTURES) ||
                !triangles->count)
            {
                fprintf(stderr, "Failed to load the cube model's data.\n");
                goto cleanup;
            }

            for (i = 0; i < NUM_TEXTURES; i++)
            {
                kelpo->rasterizer.upload_texture(&TEXTURES[i]);
            }
        }

        /* The font.*/
        FONT_TEXTURE = kelpoa_text_mesh__create_font();
        kelpo->rasterizer.upload_texture(FONT_TEXTURE);

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

    render_loop:
    while (kelpo->window.process_messages(),
           !kelpo->window.is_closing())
    {
        static float rotX = 0, rotY = 0, rotZ = 0;

        if (USER_WANTS_RENDERER_CHANGE)
        {
            /* We want to display to the user any errors that result from changing
             * (or trying to change) the renderer, so we clear away previous errors.*/
            kelpo_error_reset();
            RENDERER_CHANGE_ERROR_CODE = KELPOERR_ALL_GOOD;

            if (!use_another_renderer(kelpo, NEXT_RENDERER_NAME, &cliArgs))
            {
                fprintf(stderr, "Falling back...\n");

                /* Once we've recorded the error code associated with failing to change
                 * the renderer, we can reset Kelpo's error code queue to catch any
                 * relevant new errors.*/
                RENDERER_CHANGE_ERROR_CODE = kelpo_error_peek();
                FAILED_RENDERER_NAME = NEXT_RENDERER_NAME;
                kelpo_error_reset();

                if (!use_another_renderer(kelpo, cliArgs.rendererName, &cliArgs))
                {
                    fprintf(stderr, "Failed to restore the fallback renderer. Exiting.\n");
                    goto cleanup;
                }
            }
            
            USER_WANTS_RENDERER_CHANGE = 0;
            goto render_loop;
        }

        /* Transform the scene's triangles into screen space.*/
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
            kelpoa_text_mesh__print(screenSpaceTriangles, "Press 1-5 to set renderer", 25, 120, 255, 255, 255, 1);

            if (RENDERER_CHANGE_ERROR_CODE)
            {
                kelpoa_text_mesh__print(screenSpaceTriangles,
                                        FAILED_RENDERER_NAME,
                                        25,
                                        (cliArgs.windowHeight - (2 * kelpoa_text_mesh__character_height()) - 25),
                                        255, 0, 0,
                                        1.25);

                kelpoa_text_mesh__print(screenSpaceTriangles,
                                        kelpo_error_string(RENDERER_CHANGE_ERROR_CODE),
                                        25,
                                        (cliArgs.windowHeight - kelpoa_text_mesh__character_height() - 25),
                                        255, 0, 0,
                                        1.25);
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

    free(TEXTURES);
    free(FONT_TEXTURE);
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
