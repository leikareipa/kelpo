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
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_auxiliary/load_kac_1_0_mesh.h>
#include <kelpo_auxiliary/triangle_preparer.h>
#include <kelpo_auxiliary/matrix_44.h>
#include <kelpo_auxiliary/text_mesh.h>
#include <kelpo_auxiliary/misc.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/interface.h>
#include "../../common_src/default_window_message_handler.h"
#include "../../common_src/parse_command_line.h"

/* The default render resolution. Note: The render resolution may be modified
 * by command-line arguments.*/
static struct { unsigned width; unsigned height; unsigned bpp; } RENDER_RESOLUTION = {640, 480, 16};

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

    /* The location, in world units, of the near and far clipping planes.*/
    const float zNear = 0.1;
    const float zFar = 100;

    struct kelpo_interface_s renderer = kelpo_create_interface("opengl_1_2");
    
    uint32_t numTextures = 0;
    struct kelpo_polygon_texture_s *textures = NULL;
    struct kelpoa_generic_stack_s *triangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *worldSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpoa_generic_stack_s *screenSpaceTriangles = kelpoa_generic_stack__create(1, sizeof(struct kelpo_polygon_triangle_s));
    struct kelpo_polygon_texture_s *fontTexture = kelpoa_text_mesh__create_font();

    struct kelpoa_matrix44_s clipSpaceMatrix;
    struct kelpoa_matrix44_s screenSpaceMatrix;
    
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
                    RENDER_RESOLUTION.width = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    assert((RENDER_RESOLUTION.width != 0u) && "Invalid render width.");
                    break;
                }
                case 'h':
                {
                    RENDER_RESOLUTION.height = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    assert((RENDER_RESOLUTION.height != 0u) && "Invalid render height.");
                    break;
                }
                case 'b':
                {
                    RENDER_RESOLUTION.bpp = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                    assert((RENDER_RESOLUTION.bpp != 0u) && "Invalid render bit depth.");
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
        renderer.window.open(renderDeviceIdx,
                             RENDER_RESOLUTION.width,
                             RENDER_RESOLUTION.height,
                             RENDER_RESOLUTION.bpp);
                            
        renderer.window.set_message_handler(default_window_message_handler);

        renderer.rasterizer.upload_texture(fontTexture);
        free(fontTexture->mipLevel[0]);
        fontTexture->mipLevel[0] = NULL;
    }

    /* Load in the high-polycount model.*/
    {
        uint32_t i = 0;

        if (!kelpoa_load_kac10_mesh("apricot.kac", triangles, &textures, &numTextures) ||
            !triangles->count)
        {
            fprintf(stderr, "ERROR: Could not load the model.\n");
            return 1;
        }

        for (i = 0; i < numTextures; i++)
        {
            renderer.rasterizer.upload_texture(&textures[i]);
        }
    }

    kelpoa_matrix44__make_clip_space_matrix(&clipSpaceMatrix,
                                            KELPOA_DEG_TO_RAD(60),
                                            (RENDER_RESOLUTION.width / (float)RENDER_RESOLUTION.height),
                                            zNear,
                                            zFar);

    kelpoa_matrix44__make_screen_space_matrix(&screenSpaceMatrix,
                                              (RENDER_RESOLUTION.width / 2.0f),
                                              (RENDER_RESOLUTION.height / 2.0f));

    /* Render.*/
    while (renderer.window.is_open())
    {
        static float rotX = 0, rotY = 0, rotZ = 0;
        rotY += 0.01;

        renderer.window.process_events();

        /* Transform the scene's triangles into screen space.*/
        kelpoa_generic_stack__clear(worldSpaceTriangles);
        kelpoa_generic_stack__clear(screenSpaceTriangles);
        kelpoa_triprepr__duplicate_triangles(triangles, worldSpaceTriangles);
        kelpoa_triprepr__rotate_triangles(worldSpaceTriangles, rotX, rotY, rotZ);
        kelpoa_triprepr__translate_triangles(worldSpaceTriangles, 0, -2.5, 8.5);
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
            const unsigned fps = framerate();
            const unsigned numScreenPolys = screenSpaceTriangles->count;
            const unsigned numWorldPolys = worldSpaceTriangles->count;

            sprintf(fpsString, "FPS: %d", ((fps > 999)? 999 : fps));
            sprintf(polyString, "Polygons: %d/%d", ((numScreenPolys > 9999999)? 9999999 : numScreenPolys),
                                                   ((numWorldPolys > 9999999)? 9999999 : numWorldPolys));

            kelpoa_text_mesh__print(screenSpaceTriangles, renderer.metadata.rendererName, 25, 30, 255, 255, 255, 1);
            kelpoa_text_mesh__print(screenSpaceTriangles, polyString, 25, 60, 200, 200, 200, 1);
            kelpoa_text_mesh__print(screenSpaceTriangles, fpsString, 25, 90, 200, 200, 200, 1);
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
