/*
 * 2020 Tarpeeksi Hyvae Soft
 *
 */

#ifndef KELPO_AUXILIARY_MATRIX_44_H
#define KELPO_AUXILIARY_MATRIX_44_H

struct matrix44_s
{
    float elements[16];
};

void matrix44_multiply_two_matrices(const struct matrix44_s *const m1,
                                    const struct matrix44_s *const m2,
                                    struct matrix44_s *const dst);

void matrix44_make_rotation_matrix(struct matrix44_s *const m,
                                   float x,
                                   float y,
                                   float z);

void matrix44_make_translation_matrix(struct matrix44_s *const m,
                                      const float x,
                                      const float y,
                                      const float z);

void matrix44_make_scaling_matrix(struct matrix44_s *const m,
                                  const float x,
                                  const float y,
                                  const float z);

void matrix44_make_clip_space_matrix(struct matrix44_s *const m,
                                     const float fov,
                                     const float aspectRatio,
                                     const float zNear,
                                     const float zFar);

void matrix44_make_screen_space_matrix(struct matrix44_s *const m,
                                       const float halfWidth,
                                       const float halfHeight);


#endif
