/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * OpenGL 1.2 rasterizer for the Kelpo renderer.
 * 
 */

#ifndef KELPO_RENDERER_RASTERIZER_OPENGL_H
#define KELPO_RENDERER_RASTERIZER_OPENGL_H

struct kelpo_polygon_triangle_s;
struct kelpo_polygon_texture_s;

int kelpo_rasterizer_opengl_1_2__initialize(void);

int kelpo_rasterizer_opengl_1_2__release(void);

int kelpo_rasterizer_opengl_1_2__clear_frame(void);

int kelpo_rasterizer_opengl_1_2__upload_texture(struct kelpo_polygon_texture_s *const texture);

int kelpo_rasterizer_opengl_1_2__update_texture(struct kelpo_polygon_texture_s *const texture);

int kelpo_rasterizer_opengl_1_2__unload_textures(void);

int kelpo_rasterizer_opengl_1_2__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
                                                const unsigned numTriangles);

#endif
