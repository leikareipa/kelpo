/*
 * 2020 Tarpeeksi Hyvae Soft
 *
 */

#ifndef KELPO_AUXILIARY_TRIANGLE_PREPARER_H
#define KELPO_AUXILIARY_TRIANGLE_PREPARER_H

struct kelpoa_generic_stack_s;
struct kelpoa_matrix44_s;

/* Shallowly copies the given triangles into the destination stack. It's
 * expected that triangles' vertices aren't pointers, so that even a shallow
 * copy will duplicate them.*/
void kelpoa_triprepr__duplicate_triangles(const struct kelpoa_generic_stack_s *const triangles,
                                          struct kelpoa_generic_stack_s *const duplicatedTriangles);

/* Transforms the given triangles' vertices by the given 4-by-4 matrix.*/
void kelpoa_triprepr__transform_triangles(struct kelpoa_generic_stack_s *const triangles,
                                          struct kelpoa_matrix44_s *const matrix);

/* Rotates the given triangles' vertices around the origin 0, 0, 0. Expects the
 * triangles to be in world space.*/
void kelpoa_triprepr__rotate_triangles(struct kelpoa_generic_stack_s *const triangles,
                                       const float x,
                                       const float y,
                                       const float z);

/* Appends the given XYZ coordinate values to the given triangles' vertex
 * coordinates. Expects the triangles to be in world space.*/
void kelpoa_triprepr__translate_triangles(struct kelpoa_generic_stack_s *const triangles,
                                          const float x,
                                          const float y,
                                          const float z);

/* Multiplies the triangles' vertex coordinates by the given XYZ scale values.
 * Expects the triangles to be in world space.*/
void kelpoa_triprepr__scale_triangles(struct kelpoa_generic_stack_s *const triangles,
                                      const float x,
                                      const float y,
                                      const float z);

/* Clips the given triangles against the view frustum. Expects the triangles
 * to be in world space. The vertices will be transformed into screen space.*/
void kelpoa_triprepr__project_triangles_to_screen(const struct kelpoa_generic_stack_s *const triangles,
                                                  struct kelpoa_generic_stack_s *const screenSpaceTriangles,
                                                  const struct kelpoa_matrix44_s *const clipSpaceMatrix,
                                                  const struct kelpoa_matrix44_s *const screenSpaceMatrix);

#endif
