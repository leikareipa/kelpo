/*
 * 2020 Tarpeeksi Hyvae Soft
 *
 */

#ifndef KELPO_AUXILIARY_MATRIX_44_H
#define KELPO_AUXILIARY_MATRIX_44_H

struct kelpoa_matrix44_s
{
    float elements[16];
};

void kelpoa_matrix44__multiply_two_matrices(const struct kelpoa_matrix44_s *const m1,
                                            const struct kelpoa_matrix44_s *const m2,
                                            struct kelpoa_matrix44_s *const dst);

void kelpoa_matrix44__make_rotation_matrix(struct kelpoa_matrix44_s *const m,
                                           float x,
                                           float y,
                                           float z);

void kelpoa_matrix44__make_translation_matrix(struct kelpoa_matrix44_s *const m,
                                              const float x,
                                              const float y,
                                              const float z);

void kelpoa_matrix44__make_scaling_matrix(struct kelpoa_matrix44_s *const m,
                                          const float x,
                                          const float y,
                                          const float z);

void kelpoa_matrix44__make_clip_space_matrix(struct kelpoa_matrix44_s *const m,
                                             const float fov,
                                             const float aspectRatio,
                                             const float zNear,
                                             const float zFar);

void kelpoa_matrix44__make_screen_space_matrix(struct kelpoa_matrix44_s *const m,
                                               const float halfWidth,
                                               const float halfHeight);


#endif
