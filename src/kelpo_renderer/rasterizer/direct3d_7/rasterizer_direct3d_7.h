/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Direct3D 7 rasterizer for the Kelpo renderer.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#ifndef KELPO_RENDERER_RASTERIZER_DIRECT3D_7_H
#define KELPO_RENDERER_RASTERIZER_DIRECT3D_7_H

struct kelpo_polygon_triangle_s;
struct kelpo_polygon_texture_s;

int kelpo_rasterizer_direct3d_7__initialize(void);

int kelpo_rasterizer_direct3d_7__release(void);

int kelpo_rasterizer_direct3d_7__clear_frame(void);

int kelpo_rasterizer_direct3d_7__upload_texture(struct kelpo_polygon_texture_s *const texture);

int kelpo_rasterizer_direct3d_7__update_texture(struct kelpo_polygon_texture_s *const texture);

int kelpo_rasterizer_direct3d_7__unload_textures(void);

int kelpo_rasterizer_direct3d_7__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
                                                const unsigned numTriangles);

#endif
