/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: shiet
 * 
 * Creates a triangle mesh representing a string of ASCII text, with the character
 * symbols loaded from a texture.
 * 
 * Usage:
 * 
 *   1. Call __create_font() to initialize the font.
 * 
 *   2. Upload the font texture you received in (1) to the renderer.
 * 
 *   3. Call __print().
 * 
 */

#ifndef SHIET_EXAMPLES_COMMON_SRC_TEXT_MESH_H
#define SHIET_EXAMPLES_COMMON_SRC_TEXT_MESH_H

struct shiet_polygon_texture_s;
struct shiet_generic_data_stack_s;

/* Loads the font's character set from a file and converts it into a shiet
 * texture, which you should then upload to the renderer.*/
struct shiet_polygon_texture_s* shiet_text_mesh__create_font(void);

/* Creates a triangle mesh representing the given ASCII string and positions
 * it at the given XY coordinates. The mesh is appended to the given triangle
 * stack.*/
void shiet_text_mesh__print(const char *text,
                            unsigned posX,
                            unsigned posY,
                            struct shiet_generic_data_stack_s *const dstTriangles);

#endif
