/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 */

#include <assert.h>
#include <math.h>
#include "matrix_44.h"

void matrix44_multiply_two_matrices(const struct matrix44_s *const m1,
                                    const struct matrix44_s *const m2,
                                    struct matrix44_s *const dst)
{
    int i, j;

    assert(((m1 != m2) && (m1 != dst) && (m2 != dst)) && "Can't do in-place matrix multiplication.");

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            dst->elements[i + (j * 4)] = m1->elements[i + (0 * 4)] * m2->elements[0 + (j * 4)] +
                                         m1->elements[i + (1 * 4)] * m2->elements[1 + (j * 4)] +
                                         m1->elements[i + (2 * 4)] * m2->elements[2 + (j * 4)] +
                                         m1->elements[i + (3 * 4)] * m2->elements[3 + (j * 4)];
        }
    }

    return;
}

void matrix44_make_rotation_matrix(struct matrix44_s *const m,
                                   float x,
                                   float y,
                                   float z)
{
    struct matrix44_s rx, ry, rz, tmp;

    rx.elements[0] = 1;      rx.elements[4] = 0;       rx.elements[8]  = 0;       rx.elements[12] = 0;
    rx.elements[1] = 0;      rx.elements[5] = cos(x);  rx.elements[9]  = -sin(x); rx.elements[13] = 0;
    rx.elements[2] = 0;      rx.elements[6] = sin(x);  rx.elements[10] = cos(x);  rx.elements[14] = 0;
    rx.elements[3] = 0;      rx.elements[7] = 0;       rx.elements[11] = 0;       rx.elements[15] = 1;

    ry.elements[0] = cos(y); ry.elements[4] = 0;       ry.elements[8]  = -sin(y); ry.elements[12] = 0;
    ry.elements[1] = 0;      ry.elements[5] = 1;       ry.elements[9]  = 0;       ry.elements[13] = 0;
    ry.elements[2] = sin(y); ry.elements[6] = 0;       ry.elements[10] = cos(y);  ry.elements[14] = 0;
    ry.elements[3] = 0;      ry.elements[7] = 0;       ry.elements[11] = 0;       ry.elements[15] = 1;

    rz.elements[0] = cos(z); rz.elements[4] = -sin(z); rz.elements[8]  = 0;       rz.elements[12] = 0;
    rz.elements[1] = sin(z); rz.elements[5] = cos(z);  rz.elements[9]  = 0;       rz.elements[13] = 0;
    rz.elements[2] = 0;      rz.elements[6] = 0;       rz.elements[10] = 1;       rz.elements[14] = 0;
    rz.elements[3] = 0;      rz.elements[7] = 0;       rz.elements[11] = 0;       rz.elements[15] = 1;

    matrix44_multiply_two_matrices(&rz, &ry, &tmp);
    matrix44_multiply_two_matrices(&rx, &tmp, m);

    return;
}

void matrix44_make_translation_matrix(struct matrix44_s *const m,
                                      const float x,
                                      const float y,
                                      const float z)
{
    m->elements[0] = 1;    m->elements[4] = 0;    m->elements[8]  = 0;    m->elements[12] = x;
    m->elements[1] = 0;    m->elements[5] = 1;    m->elements[9]  = 0;    m->elements[13] = y;
    m->elements[2] = 0;    m->elements[6] = 0;    m->elements[10] = 1;    m->elements[14] = z;
    m->elements[3] = 0;    m->elements[7] = 0;    m->elements[11] = 0;    m->elements[15] = 1;

    return;
}

void matrix44_make_clip_space_matrix(struct matrix44_s *const m,
                                     const float fov,
                                     const float aspectRatio,
                                     const float zNear,
                                     const float zFar)
{
    float tanHalfFOV = tan(fov / 2);
    float zRange = zNear - zFar;

    m->elements[0] = 1.0f / (tanHalfFOV * aspectRatio); m->elements[4] = 0;                 m->elements[8] = 0;                      m->elements[12] = 0;
    m->elements[1] = 0;                                 m->elements[5] = 1.0f / tanHalfFOV; m->elements[9] = 0;                      m->elements[13] = 0;
    m->elements[2] = 0;                                 m->elements[6] = 0;                 m->elements[10] = (-zNear -zFar)/zRange; m->elements[14] = 2 * zFar * zNear / zRange;
    m->elements[3] = 0;                                 m->elements[7] = 0;                 m->elements[11] = 1;                     m->elements[15] = 0;

    return;
}

void matrix44_make_scaling_matrix(struct matrix44_s *const m,
                                  const float x,
                                  const float y,
                                  const float z)
{
    m->elements[0] = x;    m->elements[4] = 0;    m->elements[8]  = 0;    m->elements[12] = 0;
    m->elements[1] = 0;    m->elements[5] = y;    m->elements[9]  = 0;    m->elements[13] = 0;
    m->elements[2] = 0;    m->elements[6] = 0;    m->elements[10] = z;    m->elements[14] = 0;
    m->elements[3] = 0;    m->elements[7] = 0;    m->elements[11] = 0;    m->elements[15] = 1;

    return;
}

void matrix44_make_screen_space_matrix(struct matrix44_s *const m,
                                       const float halfWidth,
                                       const float halfHeight)
{
    m->elements[0] = halfWidth;    m->elements[4] = 0;             m->elements[8]  = 0;    m->elements[12] = halfWidth - 0.5f;
    m->elements[1] = 0;            m->elements[5] = -halfHeight;   m->elements[9]  = 0;    m->elements[13] = halfHeight - 0.5f;
    m->elements[2] = 0;            m->elements[6] = 0;             m->elements[10] = 1;    m->elements[14] = 0;
    m->elements[3] = 0;            m->elements[7] = 0;             m->elements[11] = 0;    m->elements[15] = 1;

    return;
}
