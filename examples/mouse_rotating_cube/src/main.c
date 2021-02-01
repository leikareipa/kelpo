/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Loads and renders a simple rotating cube model using the Kelpo renderer. The
 * cube can be rotated by moving the mouse.
 * 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <kelpo_auxiliary/triangle_preparer.h>
#include <kelpo_auxiliary/load_kac_1_0_mesh.h>
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
#include <windowsx.h>

/* Will hold any relevant command-line arguments provided by the user.*/
static struct cliparse_params_s CLI_ARGS = {0};

/* For rotating the cube. These values will be modified by mouse/keyboard input.*/
static struct { float rotX, rotY, rotZ, zoom; } CAMERA = {0, 0, 0, 4.7};

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

/* A message handler that will be attached to the Kelpo window; to monitor the
 * user's mouse and keyboard inputs.*/
static LRESULT window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int prevMousePosX = 0;
    static int prevMousePosY = 0;

    default_window_message_handler(windowHandle, message, wParam, lParam);

    switch (message)
    {
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_UP:   CAMERA.zoom -= 0.05; break;
                case VK_DOWN: CAMERA.zoom += 0.05; break;
                default: break;
            }

            break;
        }

        case WM_MOUSEMOVE:
        {
            const float deltaX = (prevMousePosX - GET_X_LPARAM(lParam));
            const float deltaY = (prevMousePosY - GET_Y_LPARAM(lParam));

            CAMERA.rotY -= (deltaX / 300.0f);
            CAMERA.rotX += (deltaY / 300.0f);

            if (deltaX || deltaY)
            {
                SetCursorPos((CLI_ARGS.windowWidth / 2), (CLI_ARGS.windowHeight / 2));

                prevMousePosX = (CLI_ARGS.windowWidth / 2);
                prevMousePosY = (CLI_ARGS.windowHeight / 2);
            }

            break;
        }

        default: break;
    }

	return 0;
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
    CLI_ARGS.rendererName = "opengl_1_1";
    CLI_ARGS.windowWidth = 1920;
    CLI_ARGS.windowHeight = 1080;
    CLI_ARGS.windowBPP = 32;
    cliparse_get_params(argc, argv, &CLI_ARGS);

    /* Initialize Kelpo.*/
    if (!kelpo_create_interface(&kelpo, CLI_ARGS.rendererName) ||
        !kelpo->window.open(CLI_ARGS.renderDeviceIdx, CLI_ARGS.windowWidth, CLI_ARGS.windowHeight, CLI_ARGS.windowBPP) ||
        !kelpo->window.set_message_handler(window_message_handler))
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

        /* The cube model.*/
        {
            if (!kelpoa_load_kac10_mesh("cube.kac", triangles, &textures, &numTextures) ||
                !triangles->count)
            {
                fprintf(stderr, "Failed to load the cube model's data.\n");
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
                                                (CLI_ARGS.windowWidth / (float)CLI_ARGS.windowHeight),
                                                0.1, 100);

        kelpoa_matrix44__make_screen_space_matrix(&screenSpaceMatrix,
                                                  (CLI_ARGS.windowWidth / 2.0f),
                                                  (CLI_ARGS.windowHeight / 2.0f));
    }

    /* Render loop.*/
    while (kelpo->window.process_messages(),
           !kelpo->window.is_closing())
    {
        /* Rotate the cube and transform its vertices into screen space.*/
        kelpoa_generic_stack__clear(worldSpaceTriangles);
        kelpoa_generic_stack__clear(screenSpaceTriangles);
        kelpoa_triprepr__duplicate_triangles(triangles, worldSpaceTriangles);
        kelpoa_triprepr__rotate_triangles(worldSpaceTriangles, CAMERA.rotX, CAMERA.rotY, CAMERA.rotZ);
        kelpoa_triprepr__translate_triangles(worldSpaceTriangles, 0, 0, CAMERA.zoom);
        kelpoa_triprepr__project_triangles_to_screen(worldSpaceTriangles,
                                                     screenSpaceTriangles,
                                                     &clipSpaceMatrix,
                                                     &screenSpaceMatrix,
                                                     0.1, 100, 1);

        /* Print the UI text.*/
        {
            /* Renderer name, and the current FPS.*/
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

            /* Usage instructions.*/
            {
                const float infoStringScale = 1.3;
                const char infoString1[] = "Mouse rotates";
                const char infoString2[] = "Up/down arrows zoom";
                const unsigned infoString1PixelLen = (strlen(infoString1) * infoStringScale * kelpoa_text_mesh__character_width());
                const unsigned infoString2PixelLen = (strlen(infoString2) * infoStringScale * kelpoa_text_mesh__character_width());

                kelpoa_text_mesh__print(screenSpaceTriangles,
                                        infoString1,
                                        ((CLI_ARGS.windowWidth / 2) - (infoString1PixelLen / 2)),
                                        (CLI_ARGS.windowHeight - 100 - 10 - kelpoa_text_mesh__character_height()),
                                        255, 255, 0, infoStringScale);

                kelpoa_text_mesh__print(screenSpaceTriangles,
                                        infoString2,
                                        ((CLI_ARGS.windowWidth / 2) - (infoString2PixelLen / 2)),
                                        (CLI_ARGS.windowHeight - 100),
                                        255, 255, 0, infoStringScale);
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
