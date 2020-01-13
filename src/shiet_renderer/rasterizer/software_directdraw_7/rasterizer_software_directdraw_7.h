/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A software rasterizer using DirectDraw 7 for the shiet renderer.
 * 
 */

#ifndef SHIET_RASTERIZER_SOFTWARE_DIRECTDRAW_7_H
#define SHIET_RASTERIZER_SOFTWARE_DIRECTDRAW_7_H

struct shiet_polygon_triangle_s;
struct shiet_polygon_texture_s;

void shiet_rasterizer_software_directdraw_7__initialize(void);

void shiet_rasterizer_software_directdraw_7__clear_frame(void);

void shiet_rasterizer_software_directdraw_7__upload_texture(struct shiet_polygon_texture_s *const texture);

void shiet_rasterizer_software_directdraw_7__update_texture(struct shiet_polygon_texture_s *const texture);

void shiet_rasterizer_software_directdraw_7__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                                            const unsigned numTriangles);

#endif
