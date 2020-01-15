/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: shiet
 * 
 * Creates a triangle mesh representing a string of ASCII text, with the character
 * symbols loaded from a texture.
 * 
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdio.h>
#include <shiet_interface/common/stdint.h>
#include <shiet_interface/polygon/texture.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include <shiet_interface/polygon/triangle/triangle_stack.h>
#include "text_mesh.h"

/* A texture that contains the font's character set. The character set is
 * expected to begin with the space ' ' character and continue on through
 * the ASCII symbols.*/
static struct shiet_polygon_texture_s *FONT_TEXTURE;

/* The dimensions of the character set texture.*/
static const unsigned FONT_TEXTURE_WIDTH = 256;
static const unsigned FONT_TEXTURE_HEIGHT = 256;

/* How many character symbols there are in the character set texture.*/
static const unsigned NUM_CHARS_PER_HORIZONTAL_LINE = 16;
static const unsigned NUM_CHARS_PER_VERTICAL_LINE = 16;

/* The dimensions of each individual character in the string mesh (not in
 * the character set texture).*/
static const unsigned CHAR_WIDTH = 32;
static const unsigned CHAR_HEIGHT = 32;

/* Creates a quad of two triangles into the given triangle stack, representing
 * the given ASCII character.*/
static void add_character(const char chr,
                          const unsigned posX,
                          const unsigned posY,
                          struct shiet_polygon_triangle_stack_s *dst)
{
    struct shiet_polygon_triangle_s leftTri, rightTri;

    /* Calculate the UV coordinates in the character set texture of this particular
     * character.*/
    const int charId = (chr - ' ');
    const float uStart = ((1.0 / NUM_CHARS_PER_HORIZONTAL_LINE) * (charId % 16));
    const float uEnd = (uStart + (1.0 / NUM_CHARS_PER_HORIZONTAL_LINE) - FLT_MIN);
    const float vStart = (1.0 / NUM_CHARS_PER_VERTICAL_LINE) * (charId / 16) + 0.5;
    const float vEnd = (vStart + (1.0 / NUM_CHARS_PER_VERTICAL_LINE) - FLT_MIN);

    assert((chr >= ' ') &&
           "Cannot represent ASCII characters that precede the space symbol.");

    memset(&leftTri, 0, sizeof(leftTri));
    memset(&rightTri, 0, sizeof(rightTri));

    leftTri.vertex[0].x = posX;
    leftTri.vertex[0].y = posY;
    leftTri.vertex[0].z = 0;
    leftTri.vertex[0].w = 1;
    leftTri.vertex[0].u = uStart;
    leftTri.vertex[0].v = vStart;
    leftTri.vertex[0].r = 255;
    leftTri.vertex[0].g = 255;
    leftTri.vertex[0].b = 255;
    leftTri.vertex[0].a = 255;

    leftTri.vertex[1].x = posX;
    leftTri.vertex[1].y = (posY + CHAR_HEIGHT);
    leftTri.vertex[1].z = 0;
    leftTri.vertex[1].w = 1;
    leftTri.vertex[1].u = uStart;
    leftTri.vertex[1].v = vEnd;
    leftTri.vertex[1].r = 255;
    leftTri.vertex[1].g = 255;
    leftTri.vertex[1].b = 255;
    leftTri.vertex[1].a = 255;

    leftTri.vertex[2].x = (posX + CHAR_WIDTH);
    leftTri.vertex[2].y = (posY + CHAR_HEIGHT);
    leftTri.vertex[2].z = 0;
    leftTri.vertex[2].w = 1;
    leftTri.vertex[2].u = uEnd;
    leftTri.vertex[2].v = vEnd;
    leftTri.vertex[2].r = 255;
    leftTri.vertex[2].g = 255;
    leftTri.vertex[2].b = 255;
    leftTri.vertex[2].a = 255;

    /**/

    rightTri.vertex[0].x = posX;
    rightTri.vertex[0].y = posY;
    rightTri.vertex[0].z = 0;
    rightTri.vertex[0].w = 1;
    rightTri.vertex[0].u = uStart;
    rightTri.vertex[0].v = vStart;
    rightTri.vertex[0].r = 255;
    rightTri.vertex[0].g = 255;
    rightTri.vertex[0].b = 255;
    rightTri.vertex[0].a = 255;

    rightTri.vertex[1].x = (posX + CHAR_WIDTH);
    rightTri.vertex[1].y = (posY + CHAR_HEIGHT);
    rightTri.vertex[1].z = 0;
    rightTri.vertex[1].w = 1;
    rightTri.vertex[1].u = uEnd;
    rightTri.vertex[1].v = vEnd;
    rightTri.vertex[1].r = 255;
    rightTri.vertex[1].g = 255;
    rightTri.vertex[1].b = 255;
    rightTri.vertex[1].a = 255;

    rightTri.vertex[2].x = (posX + CHAR_WIDTH);
    rightTri.vertex[2].y = posY;
    rightTri.vertex[2].z = 0;
    rightTri.vertex[2].w = 1;
    rightTri.vertex[2].u = uEnd;
    rightTri.vertex[2].v = vStart;
    rightTri.vertex[2].r = 255;
    rightTri.vertex[2].g = 255;
    rightTri.vertex[2].b = 255;
    rightTri.vertex[2].a = 255;

    leftTri.texture = FONT_TEXTURE;
    rightTri.texture = FONT_TEXTURE;

    shiet_tristack_push_copy(dst, &leftTri);
    shiet_tristack_push_copy(dst, &rightTri);

    return;
}

void shiet_text_mesh__print(const char *text,
                            unsigned posX,
                            unsigned posY,
                            struct shiet_polygon_triangle_stack_s *const dst)
{
    while (*text)
    {
        add_character(*text, posX, posY, dst);

        text++;
        posX += (CHAR_WIDTH / 2);
    }

    return;
}

struct shiet_polygon_texture_s* shiet_text_mesh__create_font(void)
{
    unsigned i = 0;
    FILE *const fontFile = fopen("sample-font.raw", "rb"); /* A headerless file with 256*256 RGB 888 pixels.*/

    assert(!FONT_TEXTURE && "Attempting to double initialize a font.");

    FONT_TEXTURE = malloc(sizeof(FONT_TEXTURE[0]));

    memset(FONT_TEXTURE, 0, sizeof(FONT_TEXTURE[0]));
    FONT_TEXTURE->width = FONT_TEXTURE_WIDTH;
    FONT_TEXTURE->height = FONT_TEXTURE_HEIGHT;
    FONT_TEXTURE->numMipLevels = 1;
    FONT_TEXTURE->mipLevel[0] = malloc(FONT_TEXTURE->width * FONT_TEXTURE->height * sizeof(FONT_TEXTURE->mipLevel[0]));

    /* Read the font's pixels into the texture.*/
    for (i = 0; i < (FONT_TEXTURE->width * FONT_TEXTURE->height); i++)
    {
        uint8_t color[3];

        fread(color, 1, 3, fontFile);
        assert(!ferror(fontFile));

        /* Convert the pixel into shiet's color format (RGBA 5551).*/
        FONT_TEXTURE->mipLevel[0][i] = ((color[0] <<  0) |
                                        (color[1] <<  5) |
                                        (color[2] << 10) |
                                        ((color[0]? 1 : 0) << 15));
    }

    fclose(fontFile);

    return FONT_TEXTURE;
}

static void release_font_texture(void)
{
    free(FONT_TEXTURE);
    FONT_TEXTURE = NULL;

    return;
}
