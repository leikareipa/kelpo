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
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_auxiliary/triangle_preparer.h>
#include <kelpo_auxiliary/matrix_44.h>
#include <kelpo_auxiliary/load_kac_1_0_mesh.h>
#include <kelpo_auxiliary/text_mesh.h>
#include <kelpo_auxiliary/misc.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/interface.h>
#include "../../common_src/default_window_message_handler.h"
#include "../../common_src/parse_command_line.h"

#include <windows.h>
#include <windowsx.h>

static struct cliparse_params_s cliParams = {0};

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

static float rotX = 0;
static float rotY = 0;
static float zoom = 4.7;

LRESULT window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
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
                case VK_UP:   zoom -= 0.05; break;
                case VK_DOWN: zoom += 0.05; break;
                default: break;
            }

            break;
        }

        case WM_MOUSEMOVE:
        {
            const float deltaX = (prevMousePosX - GET_X_LPARAM(lParam));
            const float deltaY = (prevMousePosY - GET_Y_LPARAM(lParam));

            rotY -= (deltaX / 300.0f);
            rotX += (deltaY / 300.0f);

            if (deltaX || deltaY)
            {
                SetCursorPos((cliParams.windowWidth / 2), (cliParams.windowHeight / 2));

                prevMousePosX = (cliParams.windowWidth / 2);
                prevMousePosY = (cliParams.windowHeight / 2);
            }

            break;
        }

        default: break;
    }

	return 0;
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
                            
        renderer.window.set_message_handler(window_message_handler);

        renderer.rasterizer.upload_texture(fontTexture);
        free(fontTexture->mipLevel[0]);
        fontTexture->mipLevel[0] = NULL;
    }

    /* Load in the cube model.*/
    {
        uint32_t i = 0;

        if (!kelpoa_load_kac10_mesh("cube.kac", triangles, &textures, &numTextures) ||
            !triangles->count)
        {
            fprintf(stderr, "ERROR: Could not load the cube model.\n");
            return 1;
        }

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
        renderer.window.process_messages();

        /* Transform the scene's triangles into screen space.*/
        kelpoa_generic_stack__clear(worldSpaceTriangles);
        kelpoa_generic_stack__clear(screenSpaceTriangles);
        kelpoa_triprepr__duplicate_triangles(triangles, worldSpaceTriangles);
        kelpoa_triprepr__rotate_triangles(worldSpaceTriangles, rotX, rotY, 0);
        kelpoa_triprepr__translate_triangles(worldSpaceTriangles, 0, 0, zoom);
        kelpoa_triprepr__project_triangles_to_screen(worldSpaceTriangles,
                                                     screenSpaceTriangles,
                                                     &clipSpaceMatrix,
                                                     &screenSpaceMatrix,
                                                     zNear,
                                                     zFar,
                                                     1);

        /* Print the UI text.*/
        {
            /* Name of the renderer; and the current FPS.*/
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

            /* Usage instructions.*/
            {
                const float infoStringScale = 1.3;
                const char infoString1[] = "Mouse rotates";
                const char infoString2[] = "Up/down arrows zoom";
                const unsigned infoString1PixelLen = (strlen(infoString1) * infoStringScale * kelpoa_text_mesh__character_width());
                const unsigned infoString2PixelLen = (strlen(infoString2) * infoStringScale * kelpoa_text_mesh__character_width());

                kelpoa_text_mesh__print(screenSpaceTriangles,
                                        infoString1,
                                        ((cliParams.windowWidth / 2) - (infoString1PixelLen / 2)),
                                        (cliParams.windowHeight - 100 - 10 - kelpoa_text_mesh__character_height()),
                                        255, 255, 0, infoStringScale);

                kelpoa_text_mesh__print(screenSpaceTriangles,
                                        infoString2,
                                        ((cliParams.windowWidth / 2) - (infoString2PixelLen / 2)),
                                        (cliParams.windowHeight - 100),
                                        255, 255, 0, infoStringScale);
            }
        }

        renderer.rasterizer.clear_frame();
        renderer.rasterizer.draw_triangles(screenSpaceTriangles->data,
                                           screenSpaceTriangles->count);

        renderer.window.flip_surface();
    }

    /* Release any leftover memory.*/
    {
        /* Release textures.*/
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
        }

        kelpoa_generic_stack__free(triangles);
        kelpoa_generic_stack__free(worldSpaceTriangles);
        kelpoa_generic_stack__free(screenSpaceTriangles);
    }

    return 0;
}
