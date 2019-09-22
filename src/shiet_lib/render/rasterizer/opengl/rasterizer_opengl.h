#ifndef SHIET_RASTERIZER_OPENGL_H_
#define SHIET_RASTERIZER_OPENGL_H_

struct shiet_polygon_triangle_s;
struct shiet_polygon_texture_s;

void shiet_rasterizer_opengl__clear_frame(void);

void shiet_rasterizer_opengl__upload_texture(struct shiet_polygon_texture_s *const texture);

void shiet_rasterizer_opengl__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                             const unsigned numTriangles);

#endif
