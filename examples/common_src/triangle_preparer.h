/*
 * 2020 Tarpeeksi Hyvae Soft
 *
 */

#ifndef KELPO_EXAMPLES_COMMON_SRC_TRIANGLE_PREPARER_H
#define KELPO_EXAMPLES_COMMON_SRC_TRIANGLE_PREPARER_H

struct kelpo_generic_stack_s;
struct matrix44_s;

/* Shallowly copies the given triangles into the destination stack. It's
 * expected that triangles' vertices aren't pointers, so that even a shallow
 * copy will duplicate them.*/
void triprepr_duplicate_triangles(const struct kelpo_generic_stack_s *const triangles,
                                  struct kelpo_generic_stack_s *const duplicatedTriangles);

/* Transforms the given triangles' vertices by the given 4-by-4 matrix.*/
void triprepr_transform_triangles(struct kelpo_generic_stack_s *const triangles,
                                  struct matrix44_s *const matrix);

/* Rotates the given triangles' vertices around the origin 0, 0, 0. Expects the
 * triangles to be in world space.*/
void triprepr_rotate_triangles(struct kelpo_generic_stack_s *const triangles,
                               const float x,
                               const float y,
                               const float z);

/* Appends the given XYZ coordinate values to the given triangles' vertex
 * coordinates. Expects the triangles to be in world space.*/
void triprepr_translate_triangles(struct kelpo_generic_stack_s *const triangles,
                                  const float x,
                                  const float y,
                                  const float z);

/* Multiplies the triangles' vertex coordinates by the given XYZ scale values.
 * Expects the triangles to be in world space.*/
void triprepr_scale_triangles(struct kelpo_generic_stack_s *const triangles,
                              const float x,
                              const float y,
                              const float z);

/* Clips the given triangles against the view frustum. Expects the triangles
 * to be in world space. The vertices will be transformed into screen space.*/
void triprepr_project_triangles_to_screen(const struct kelpo_generic_stack_s *const triangles,
                                          struct kelpo_generic_stack_s *const screenSpaceTriangles,
                                          const struct matrix44_s *const clipSpaceMatrix,
                                          const struct matrix44_s *const screenSpaceMatrix);

#endif
