/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: shiet
 * 
 * A triangle rasterizer for a software renderer.
 * 
 * Usage:
 * 
 *   1. Call __set_target_pixel_buffer() to tell the rasterizer which pixel buffer
 *      to render into.
 * 
 *   2. Call __fill() for each triangle you want rendered into the pixel buffer
 *      set in (1). The triangle's vertices must be in screen space and clipped
 *      against the viewport.
 * 
 */

#ifndef SHIET_RENDERER_RASTERIZER_SOFTWARE_TRIANGLE_FILLER_H
#define SHIET_RENDERER_RASTERIZER_SOFTWARE_TRIANGLE_FILLER_H

#include <shiet_interface/common/stdint.h>

struct shiet_polygon_triangle_s;

/* Assign the unit's local variables to point to the given pixel buffer target
 * for rendering, along with the buffer's width in bytes and its pixel color
 * format. The color format is of enum SHIET_COLOR_FMT_xxxx.*/
void shiet_rasterizer_software_triangle_filler__set_target_pixel_buffer(uint8_t *const pixelBuffer,
                                                                        const unsigned pixelBufferWidth,
                                                                        const unsigned pixelColorFormat);

/* Rasterize the given triangle into the pixel buffer set by the most recent
 * call to __set_target_pixel_buffer(). The triangle's vertices must be in screen
 * space and clipped against the viewport.*/
void shiet_rasterizer_software_triangle_filler__fill(struct shiet_polygon_triangle_s *const triangle);

#endif
