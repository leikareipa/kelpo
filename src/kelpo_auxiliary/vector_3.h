/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 *
 */

#ifndef KELPO_AUXILIARY_VECTOR_3_H
#define KELPO_AUXILIARY_VECTOR_3_H

struct kelpoa_vector3_s
{
    float x, y, z;
};

struct kelpoa_matrix44_s;

float kelpoa_vector3__dot(const struct kelpoa_vector3_s *a,
                          const struct kelpoa_vector3_s *b);

struct kelpoa_vector3_s kelpoa_vector3__cross(const struct kelpoa_vector3_s *a,
                                              const struct kelpoa_vector3_s *b);

void kelpoa_vector3__normalize(struct kelpoa_vector3_s *v);

void kelpoa_vector3__transform(struct kelpoa_vector3_s *v,
                               struct kelpoa_matrix44_s *matrix);

#endif
