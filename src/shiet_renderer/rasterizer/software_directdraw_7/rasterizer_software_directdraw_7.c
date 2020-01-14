/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A software rasterizer using DirectDraw 7 for the shiet renderer.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of shiet.
 * 
 */

#include <assert.h>
#include <stdio.h>
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

enum
{
    SHIET_COLOR_FMT_UNKNOWN = 0,
    SHIET_COLOR_FMT_RGBA_8888,
    SHIET_COLOR_FMT_RGB_565,
    SHIET_COLOR_FMT_RGB_555
};

static unsigned determine_pixel_color_format(const LPDDPIXELFORMAT pixelFormat)
{
    /* We'll assume that DirectDraw is allowed to have only these three types of
     * color formats in 16/32-bit color modes.*/
    if (pixelFormat->dwGBitMask == 0x7e0) return SHIET_COLOR_FMT_RGB_565;
    if (pixelFormat->dwGBitMask == 0x3e0) return SHIET_COLOR_FMT_RGB_555;
    if (pixelFormat->dwRBitMask == 0xff0000) return SHIET_COLOR_FMT_RGBA_8888;

    assert(0 && "Software w/ DirectDraw 7: Unknown back buffer pixel format.");
}

void shiet_rasterizer_software_directdraw_7__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                                            const unsigned numTriangles)
{
    uint8_t *pixels = NULL; /* Pointer to the beginning of the render buffer we'll draw into.*/
    unsigned i = 0, v = 0;
    unsigned surfaceWidth = 0;
    unsigned colorFormat = SHIET_COLOR_FMT_UNKNOWN; /* Color format of the render buffer's pixels.*/

    /* Plots a pixel of the given r,g,b (0-255 each) color into the given x,y
     * coordinates in the 'pixels' pixel buffer. Assumes 'pixels' is a pointer
     * to a 1D uint8_t array of pixels (each of which, depending on their color
     * format, might span multiple adjacent elements in the array), 'surfaceWidth'
     * gives the number of bytes per line in the pixel array, and 'colorFormat'
     * identifies the bit-level color channel layout of a single pixel.*/
    #define PUT_PIXEL(x, y, r, g, b)\
    {\
        switch (colorFormat)\
        {\
            case SHIET_COLOR_FMT_RGBA_8888: ((uint32_t*)pixels)[(x) + (y) * (surfaceWidth / 4)] = (((r) << 16) | ((g) << 8) | (b)); break;\
            case SHIET_COLOR_FMT_RGB_565:   ((uint16_t*)pixels)[(x) + (y) * (surfaceWidth / 2)] = (((r) << 11) | ((g) << 5) | (b)); break;\
            case SHIET_COLOR_FMT_RGB_555:   ((uint16_t*)pixels)[(x) + (y) * (surfaceWidth / 2)] = (((r) << 10) | ((g) << 5) | (b)); break;\
            default: assert(0 && "Software w/ DirectDraw 7: Unknown pixel format.");\
        }\
    }

    /* Lock the render surface to allow direct pixel manipulation.*/
    {
        DDSURFACEDESC2 surfaceDesc;

        if (!shiet_surface_directdraw_7__lock_surface(&surfaceDesc) ||
            !surfaceDesc.lpSurface)
        {
            return;
        }

        pixels = (uint8_t*)surfaceDesc.lpSurface;
        surfaceWidth = surfaceDesc.lPitch;
        colorFormat = determine_pixel_color_format(&surfaceDesc.ddpfPixelFormat);
    }

    for (i = 0; i < numTriangles; i++)
    {
        for (v = 0; v < 3; v++)
        {
            PUT_PIXEL((int)triangles[i].vertex[v].x,
                      (int)triangles[i].vertex[v].y,
                      255, 255, 0);
        }
    }

    shiet_surface_directdraw_7__unlock_surface();

    #undef PUT_PIXEL

    return;
}
