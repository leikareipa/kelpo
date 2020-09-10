/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 *
 */

#ifndef KELPO_AUXILIARY_VECTOR_3D_H
#define KELPO_AUXILIARY_VECTOR_3D_H

struct kelpoa_vector3d_s
{
    double x, y, z;
};

struct kelpoa_matrix44_s;

struct kelpoa_vector3d_s kelpoa_vector3d(const double x,
                                         const double y,
                                         const double z);

double kelpoa_vector3d__dot(const struct kelpoa_vector3d_s *a,
                            const struct kelpoa_vector3d_s *b);

struct kelpoa_vector3d_s kelpoa_vector3d__cross(const struct kelpoa_vector3d_s *a,
                                                const struct kelpoa_vector3d_s *b);

void kelpoa_vector3d__normalize(struct kelpoa_vector3d_s *v);

void kelpoa_vector3d__transform(struct kelpoa_vector3d_s *v,
                                struct kelpoa_matrix44_s *matrix);

#endif
