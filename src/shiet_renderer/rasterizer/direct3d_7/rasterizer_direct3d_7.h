/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Direct3D 7 rasterizer for the shiet renderer.
 * 
 * Note: The Direct3D 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of shiet.
 * 
 */

#ifndef SHIET_RENDERER_RASTERIZER_DIRECT3D_7_H
#define SHIET_RENDERER_RASTERIZER_DIRECT3D_7_H

struct shiet_polygon_triangle_s;
struct shiet_polygon_texture_s;

void shiet_rasterizer_direct3d_7__initialize(void);

void shiet_rasterizer_direct3d_7__clear_frame(void);

void shiet_rasterizer_direct3d_7__upload_texture(struct shiet_polygon_texture_s *const texture);

void shiet_rasterizer_direct3d_7__update_texture(struct shiet_polygon_texture_s *const texture);

void shiet_rasterizer_direct3d_7__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                                 const unsigned numTriangles);

#endif
