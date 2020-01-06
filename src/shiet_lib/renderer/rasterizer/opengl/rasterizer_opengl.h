/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * OpenGL 1.2 rasterizer for the shiet renderer.
 * 
 */

#ifndef SHIET_RASTERIZER_OPENGL_H
#define SHIET_RASTERIZER_OPENGL_H

struct shiet_polygon_triangle_s;
struct shiet_polygon_texture_s;

void shiet_rasterizer_opengl__initialize(void);

void shiet_rasterizer_opengl__clear_frame(void);

void shiet_rasterizer_opengl__upload_texture(struct shiet_polygon_texture_s *const texture);

void shiet_rasterizer_opengl__update_texture(struct shiet_polygon_texture_s *const texture);

void shiet_rasterizer_opengl__draw_triangles(const struct shiet_polygon_triangle_s *const triangles,
                                             const unsigned numTriangles);

#endif
