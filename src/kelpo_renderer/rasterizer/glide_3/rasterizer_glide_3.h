/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x rasterizer for the Kelpo renderer.
 * 
 */

#ifndef KELPO_RENDERER_RASTERIZER_GLIDE3_H
#define KELPO_RENDERER_RASTERIZER_GLIDE3_H

struct kelpo_polygon_triangle_s;
struct kelpo_polygon_texture_s;

void kelpo_rasterizer_glide_3__initialize(void);

void kelpo_rasterizer_glide_3__release(void);

void kelpo_rasterizer_glide_3__clear_frame(void);

void kelpo_rasterizer_glide_3__upload_texture(struct kelpo_polygon_texture_s *const texture);

void kelpo_rasterizer_glide_3__update_texture(struct kelpo_polygon_texture_s *const texture);

void kelpo_rasterizer_glide_3__unload_textures(void);

void kelpo_rasterizer_glide_3__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
                                              const unsigned numTriangles);

#endif
