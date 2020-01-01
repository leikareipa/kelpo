/*
 * Tarpeeksi Hyvae Soft 2019
 *
 */

#ifndef SHIET_EXAMPLES_COMMON_TRANSFORM_AND_ROTATE_TRIANGLES_H
#define SHIET_EXAMPLES_COMMON_TRANSFORM_AND_ROTATE_TRIANGLES_H

struct shiet_polygon_triangle_s;

/* Set up screen-space-related matrices.*/
void trirot_initialize_screen_geometry(const unsigned renderWidth, const unsigned renderHeight);

/* Transform the given triangles into screen space and rotate them about the
 * origin. The transformed triangles will be placed in the given destination
 * buffer (the operation is not in-place).*/
unsigned trirot_transform_and_rotate_triangles(struct shiet_polygon_triangle_s *const triangles,
                                               const unsigned numTriangles,
                                               struct shiet_polygon_triangle_s *const dst,
                                               const float rotX, const float rotY, const float rotZ,
                                               const float cameraDistance);

#endif
