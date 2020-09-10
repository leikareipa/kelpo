/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#include <math.h>
#include <kelpo_auxiliary/vector_3d.h>
#include <kelpo_auxiliary/matrix_44.h>

struct kelpoa_vector3d_s kelpoa_vector3d(const double x,
                                         const double y,
                                         const double z)
{
    struct kelpoa_vector3d_s v;

    v.x = x;
    v.y = y;
    v.z = z;

    return v;
}

double kelpoa_vector3d__dot(const struct kelpoa_vector3d_s *a,
                            const struct kelpoa_vector3d_s *b)
{
    return ((a->x * b->x) + (a->y * b->y) + (a->z * b->z));
}

struct kelpoa_vector3d_s kelpoa_vector3d__cross(const struct kelpoa_vector3d_s *a,
                                                const struct kelpoa_vector3d_s *b)
{
    struct kelpoa_vector3d_s result;

    result.x = ((a->y * b->z) - (a->z * b->y));
    result.y = ((a->z * b->x) - (a->x * b->z));
    result.z = ((a->x * b->y) - (a->y * b->x));

    return result;
}

void kelpoa_vector3d__normalize(struct kelpoa_vector3d_s *v)
{
    const double sn = ((v->x * v->x) + (v->y * v->y) + (v->z * v->z));

    if ((sn != 0) && (sn != 1))
    {
        const double inv = (1.0 / sqrt(sn));

        v->x *= inv;
        v->y *= inv;
        v->z *= inv;
    }

    return;
}

void kelpoa_vector3d__transform(struct kelpoa_vector3d_s *v,
                                struct kelpoa_matrix44_s *matrix)
{
    const double x_ = ((matrix->elements[0] * v->x) + (matrix->elements[4] * v->y) + (matrix->elements[ 8] * v->z));
    const double y_ = ((matrix->elements[1] * v->x) + (matrix->elements[5] * v->y) + (matrix->elements[ 9] * v->z));
    const double z_ = ((matrix->elements[2] * v->x) + (matrix->elements[6] * v->y) + (matrix->elements[10] * v->z));

    v->x = x_;
    v->y = y_;
    v->z = z_;

    return;
}
