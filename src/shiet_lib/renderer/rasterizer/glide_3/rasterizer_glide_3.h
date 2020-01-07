/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x rasterizer for the shiet renderer.
 * 
 */

#ifndef SHIET_RASTERIZER_GLIDE3_H
#define SHIET_RASTERIZER_GLIDE3_H

struct shiet_polygon_triangle_s;
struct shiet_polygon_texture_s;

void shiet_rasterizer_glide_3__initialize(void);

void shiet_rasterizer_glide_3__clear_frame(void);

void shiet_rasterizer_glide_3__upload_texture(struct shiet_polygon_texture_s *const texture);

void shiet_rasterizer_glide_3__update_texture(struct shiet_polygon_texture_s *const texture);

void shiet_rasterizer_glide_3__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                             const unsigned numTriangles);

#endif
