/*
 * Tarpeeksi Hyvae Soft 2019
 *
 */

#ifndef KELPO_EXAMPLES_COMMON_SRC_TRANSFORM_AND_ROTATE_TRIANGLES_H
#define KELPO_EXAMPLES_COMMON_SRC_TRANSFORM_AND_ROTATE_TRIANGLES_H

struct kelpo_generic_stack_s;
struct kelpo_polygon_triangle_s;

/* Set up screen-space-related matrices.*/
void trirot_initialize_screen_geometry(const unsigned renderWidth, const unsigned renderHeight);

/* Transform the given triangles into screen space, translating their origin to
 * the given base XYZ position and rotating them about that origin by the given
 * XYZ amount. The transformed triangles will be placed in the given destination
 * stack (the transformation is not in-place).*/
void trirot_transform_and_rotate_triangles(struct kelpo_generic_stack_s *const triangles,
                                           struct kelpo_generic_stack_s *const dstTriangles,
                                           const float basePosX, const float basePosY, const float basePosZ,
                                           const float rotX, const float rotY, const float rotZ);

#endif
