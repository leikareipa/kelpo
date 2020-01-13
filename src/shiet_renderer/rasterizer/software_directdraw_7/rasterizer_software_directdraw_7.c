/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A software rasterizer using DirectDraw 7 for the shiet renderer.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of shiet.
 * 
 */

#include <shiet_renderer/surface/directdraw_7/surface_directdraw_7.h>
#include <shiet_renderer/rasterizer/software_directdraw_7/rasterizer_software_directdraw_7.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include <shiet_interface/polygon/texture.h>

void shiet_rasterizer_software_directdraw_7__initialize(void)
{
    /* TODO.*/

    return;
}

void shiet_rasterizer_software_directdraw_7__clear_frame(void)
{
    uint16_t *pixels = NULL; /* Expected pixel format: ARGB 1555 (16-bit).*/
    unsigned surfaceWidth = 0;
    unsigned surfaceHeight = 0;

    /* Lock the render surface to allow direct pixel manipulation.*/
    {
        DDSURFACEDESC2 surfaceDesc;

        if (!shiet_surface_directdraw_7__lock_surface(&surfaceDesc) ||
            !surfaceDesc.lpSurface)
        {
            return;
        }

        pixels = (uint16_t*)surfaceDesc.lpSurface;
        surfaceWidth = (surfaceDesc.lPitch / sizeof(pixels[0]));
        surfaceHeight = surfaceDesc.dwHeight;
    }

    memset(pixels, 0, (surfaceWidth * surfaceHeight * sizeof(pixels[0])));

    shiet_surface_directdraw_7__unlock_surface();

    return;
}

void shiet_rasterizer_software_directdraw_7__upload_texture(struct shiet_polygon_texture_s *const texture)
{
    /* TODO.*/
    
    return;
}

void shiet_rasterizer_software_directdraw_7__update_texture(struct shiet_polygon_texture_s *const texture)
{
    /* TODO.*/

    return;
}

void shiet_rasterizer_software_directdraw_7__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                               const unsigned numTriangles)
{
    unsigned i = 0;
    uint16_t *pixels = NULL; /* Expected pixel format: ARGB 1555 (16-bit).*/
    unsigned surfaceWidth = 0;

    /* Lock the render surface to allow direct pixel manipulation.*/
    {
        DDSURFACEDESC2 surfaceDesc;

        if (!shiet_surface_directdraw_7__lock_surface(&surfaceDesc) ||
            !surfaceDesc.lpSurface)
        {
            return;
        }

        pixels = (uint16_t*)surfaceDesc.lpSurface;
        surfaceWidth = (surfaceDesc.lPitch / sizeof(pixels[0]));
    }

    for (i = 0; i < numTriangles; i++)
    {
        unsigned v = 0;

        for (v = 0; v < 3; v++)
        {
            pixels[(int)triangles[i].vertex[v].x + (int)triangles[i].vertex[v].y * surfaceWidth] = 0xffe0;
        }
    }

    shiet_surface_directdraw_7__unlock_surface();

    return;
}
