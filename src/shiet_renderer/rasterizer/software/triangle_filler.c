/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: shiet
 * 
 * A triangle rasterizer for a software renderer.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <shiet_renderer/rasterizer/software/triangle_filler.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include <shiet_interface/polygon/texture.h>
#include <shiet_interface/polygon/vertex.h>
#include <shiet_interface/common/globals.h>

static uint8_t *PIXEL_BUFFER; /* The pixel buffer we'll rasterize into.*/
static unsigned PIXEL_BUFFER_WIDTH;
static unsigned PIXEL_COLOR_FORMAT;
static struct shiet_polygon_triangle_s *CURRENT_TRIANGLE;

void shiet_rasterizer_software_triangle_filler__set_target_pixel_buffer(uint8_t *const pixelBuffer,
                                                                        const unsigned pixelBufferWidth,
                                                                        const unsigned pixelColorFormat)
{
    PIXEL_BUFFER = pixelBuffer;
    PIXEL_BUFFER_WIDTH = pixelBufferWidth;
    PIXEL_COLOR_FORMAT = pixelColorFormat;
    return;
}

static void sort_tri_verts_by_height(struct shiet_polygon_vertex_s **high,
                                     struct shiet_polygon_vertex_s **mid,
                                     struct shiet_polygon_vertex_s **low)
{
    #define SWAP_VERTICES(v1, v2) struct shiet_polygon_vertex_s *t = v1;\
                                  v1 = v2;\
                                  v2 = t;

    if ((*low)->y < (*mid)->y)
    {
        SWAP_VERTICES((*low), (*mid));
    }
    if ((*mid)->y < (*high)->y)
    {
        SWAP_VERTICES((*mid), (*high));
    }
    if ((*low)->y < (*mid)->y)
    {
        SWAP_VERTICES((*low), (*mid));
    }

    #undef SWAP_VERTICES

    return;
}

/* Split the triangle into two straight-based triangles, one pointing up and one
 * pointing down.*/
static void split_tri(struct shiet_polygon_vertex_s *split,
                      const struct shiet_polygon_vertex_s *high,
                      const struct shiet_polygon_vertex_s *mid,
                      const struct shiet_polygon_vertex_s *low)
{
    float splitRatio = ((mid->y - high->y) / (float)(low->y - high->y));

    split->x = (high->x + ((low->x - high->x) * splitRatio));
    split->y = mid->y;
    split->u = LERP(high->u, low->u, splitRatio);
    split->v = LERP(high->v, low->v, splitRatio);

    return;
}

static void fill_tri_row(const int32_t row,
                         int32_t startX,
                         int32_t endX,
                         float leftU,
                         float leftV,
                         const float rightU,
                         const float rightV)
{
    int32_t x;
    int32_t width;
    float uDelta;
    float vDelta;

    if ((endX - startX) <= 0)
    {
        return;
    }

    width = endX - startX;
    uDelta = ((rightU - leftU) / (width + 1));
    vDelta = ((rightV - leftV) / (width + 1));

    /* Plots a pixel of the given r,g,b (0-255 each) color into the given x,y
     * coordinates in the 'PIXEL_BUFFER' pixel buffer. Assumes 'PIXEL_BUFFER' is
     * a pointer to a 1D uint8_t array of pixels (each of which, depending on
     * their color format, might span multiple adjacent elements in the array),
     * 'PIXEL_BUFFER_WIDTH' gives the number of bytes per line in the pixel array,
     * and 'PIXEL_COLOR_FORMAT' identifies the bit-level color channel layout of
     * a single pixel.*/
    #define PUT_PIXEL(x, y, r, g, b)\
    {\
        switch (PIXEL_COLOR_FORMAT)\
        {\
            case SHIET_COLOR_FMT_RGBA_8888: ((uint32_t*)PIXEL_BUFFER)[(x) + (y) * (PIXEL_BUFFER_WIDTH / 4)] = (((r) << 16) | ((g) << 8) | (b)); break;\
            case SHIET_COLOR_FMT_RGB_565:   ((uint16_t*)PIXEL_BUFFER)[(x) + (y) * (PIXEL_BUFFER_WIDTH / 2)] = ((((r)>>3) << 11) | (((g)>>2) << 5) | ((b)>>3)); break;\
            case SHIET_COLOR_FMT_RGB_555:   ((uint16_t*)PIXEL_BUFFER)[(x) + (y) * (PIXEL_BUFFER_WIDTH / 2)] = ((((r)>>3) << 10) | (((g)>>3) << 5) | ((b)>>3)); break;\
            default: assert(0 && "Software w/ DirectDraw 7: Unknown pixel format.");\
        }\
    }

    /* Draw the pixels.*/
    for (x = startX; x <= endX; x++)
    {
        uint8_t color[3] = {CURRENT_TRIANGLE->vertex[0].r,
                            CURRENT_TRIANGLE->vertex[0].g,
                            CURRENT_TRIANGLE->vertex[0].b};

        if (CURRENT_TRIANGLE->texture)
        {
            uint16_t texelColor;
            unsigned u = (int)leftU;
            unsigned v = (int)leftV;

            /* Wrap the UV coordinates to the range [0,width). Assumes texture
             * dimensions are power-of-two.*/
            u = (u & (CURRENT_TRIANGLE->texture->width - 1));
            v = (v & (CURRENT_TRIANGLE->texture->height - 1));

            texelColor = CURRENT_TRIANGLE->texture->mipLevel[0][u + v * CURRENT_TRIANGLE->texture->width];

            /* Texel colors are in BGRA 5551 format.*/
            color[0] = (((texelColor >> 10) & 0x1f) * 8);
            color[1] = (((texelColor >>  5) & 0x1f) * 8);
            color[2] = (((texelColor >>  0) & 0x1f) * 8);
        }

        PUT_PIXEL(x, row, color[0], color[1], color[2]);

        leftU += uDelta;
        leftV += vDelta;
    }

    #undef PUT_PIXEL

    return;
}

/* Rasterizes into the frame buffer the given flat-based triangle formed by the
 * three given vertices. Expects the vertices to be in screen space.*/
static void fill_tri_part(struct shiet_polygon_vertex_s *peak,
                          struct shiet_polygon_vertex_s *base1,
                          struct shiet_polygon_vertex_s *base2)
{
    int32_t startRow, endRow, y, height, flipped = 0;
    struct shiet_polygon_vertex_s *left = base2;
    struct shiet_polygon_vertex_s *right = base1;

    /* For interpolating parameters across each horizontal pixel span.*/
    float pleft, pright, dLeft, dRight, leftU, leftV, rightU, rightV, dLeftU, dLeftV, dRightU, dRightV;

    /* Figure out which of the base's vertices is on the left and which on the right.*/
    if (base1->x < base2->x)
    {
        left = base1;
        right = base2;
    }

    startRow = peak->y;
    endRow = base1->y;

    /* Flip depending on whether the peak vertex is above or below the base.*/
    if (startRow > endRow)
    {
        int32_t temp = startRow;
        startRow = endRow;
        endRow = temp;

        /* Don't draw the base row twice; i.e. skip it for the down-triangle.*/
        startRow++;

        flipped = 1;
    }

    if ((startRow == endRow) ||
        ((endRow - startRow) <= 0))
    {
        return;
    }

    height = (endRow - startRow);

    if (flipped)
    {
        pleft = left->x;
        pright = right->x;
        dLeft = ((peak->x - left->x) / (height + 1));
        dRight = ((peak->x - right->x) / (height + 1));
        leftU = left->u;
        leftV = left->v;
        rightU = right->u;
        rightV = right->v;
        dLeftU = ((peak->u - left->u) / (height + 1));
        dLeftV = ((peak->v - left->v) / (height + 1));
        dRightU = ((peak->u - right->u) / (height + 1));
        dRightV = ((peak->v - right->v) / (height + 1));
    }
    else
    {
        pleft = peak->x;
        pright = peak->x;
        dLeft = ((left->x - peak->x) / (height + 1));
        dRight = ((right->x - peak->x) / (height + 1));
        leftU = peak->u;
        leftV = peak->v;
        rightU = peak->u;
        rightV = peak->v;
        dLeftU = ((left->u - peak->u) / (height + 1));
        dLeftV = ((left->v - peak->v) / (height + 1));
        dRightU = ((right->u - peak->u) / (height + 1));
        dRightV = ((right->v - peak->v) / (height + 1));
    }

    if (CURRENT_TRIANGLE->texture)
    {
        leftU *= CURRENT_TRIANGLE->texture->width;
        leftV *= CURRENT_TRIANGLE->texture->height;
        rightU *= CURRENT_TRIANGLE->texture->width;
        rightV *= CURRENT_TRIANGLE->texture->height;
        dLeftU *= CURRENT_TRIANGLE->texture->width;
        dLeftV *= CURRENT_TRIANGLE->texture->height;
        dRightU *= CURRENT_TRIANGLE->texture->width;
        dRightV *= CURRENT_TRIANGLE->texture->height;
    }

    for (y = startRow; y <= endRow; y++)
    {
        pleft += dLeft;
        pright += dRight;
        leftU += dLeftU;
        leftV += dLeftV;
        rightU += dRightU;
        rightV += dRightV;

        fill_tri_row(y, pleft, pright, leftU, leftV, rightU, rightV);
    }

    return;
}

void shiet_rasterizer_software_triangle_filler__fill(struct shiet_polygon_triangle_s *const triangle)
{
    struct shiet_polygon_vertex_s split;
    struct shiet_polygon_vertex_s *high = &triangle->vertex[0],
                                  *mid  = &triangle->vertex[1],
                                  *low  = &triangle->vertex[2];

    CURRENT_TRIANGLE = triangle;

    /* We'll split the triangle into two flat-based triangles, then rasterize
     * the two halves separately.*/
    sort_tri_verts_by_height(&high, &mid, &low);
    split_tri(&split, high, mid, low);
    fill_tri_part(high, mid, &split); /* Up triangle.*/
    fill_tri_part(low, mid, &split);  /* Down triangle.*/

    return;
}
