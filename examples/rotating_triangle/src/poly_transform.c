/*
 * Tarpeeksi Hyvae Soft 2019
 * 
 * Software: shiet render example 1
 *
 * Portions of this file have been adapted from 4-by-4 matrix manipulation code written
 * by Benny Bobaganoosh (https://github.com/BennyQBD/3DSoftwareRenderer):
 * {
 *      Copyright (c) 2014, Benny Bobaganoosh
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are met:
 *
 *      1. Redistributions of source code must retain the above copyright notice, this
 *          list of conditions and the following disclaimer.
 *      2. Redistributions in binary form must reproduce the above copyright notice,
 *          this list of conditions and the following disclaimer in the documentation
 *          and/or other materials provided with the distribution.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *      ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *      WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *      DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *      ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *      (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *      LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *      ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * }
 *
 */

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <shiet/polygon/triangle/triangle.h>
#include <shiet/polygon/vertex.h>
#include <shiet/common/globals.h>
#include "poly_transform.h"

static const float NEAR_CLIP = 1;
static const float FAR_CLIP = 2000;

static matrix44_s SCREEN_SPACE_MAT;
static matrix44_s PERSP_MAT;

static void transform_vert(struct shiet_polygon_vertex_s *const v, const matrix44_s *const m)
{
    float x0 = ((m->data[0] * v->x) + (m->data[4] * v->y) + (m->data[8] * v->z) + (m->data[12] * v->w));
    float y0 = ((m->data[1] * v->x) + (m->data[5] * v->y) + (m->data[9] * v->z) + (m->data[13] * v->w));
    float z0 = ((m->data[2] * v->x) + (m->data[6] * v->y) + (m->data[10] * v->z) + (m->data[14] * v->w));
    float w0 = ((m->data[3] * v->x) + (m->data[7] * v->y) + (m->data[11] * v->z) + (m->data[15] * v->w));

    v->x = x0;
    v->y = y0;
    v->z = z0;
    v->w = w0;

    return;
}

static void mul_two_mats(const matrix44_s *const m1, const matrix44_s *const m2, matrix44_s *const dst)
{
    int i, j;

    assert(((m1 != m2) && (m1 != dst) && (m2 != dst)) && "Can't do in-place matrix multiplication.");

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            dst->data[i + (j * 4)] = m1->data[i + (0 * 4)] * m2->data[0 + (j * 4)] +
                                     m1->data[i + (1 * 4)] * m2->data[1 + (j * 4)] +
                                     m1->data[i + (2 * 4)] * m2->data[2 + (j * 4)] +
                                     m1->data[i + (3 * 4)] * m2->data[3 + (j * 4)];
        }
    }

    return;
}

static void make_rot_mat(matrix44_s *const m, float x, float y, float z)
{
    matrix44_s rx, ry, rz, tmp;

    rx.data[0] = 1;      rx.data[4] = 0;       rx.data[8]  = 0;       rx.data[12] = 0;
    rx.data[1] = 0;      rx.data[5] = cos(x);  rx.data[9]  = -sin(x); rx.data[13] = 0;
    rx.data[2] = 0;      rx.data[6] = sin(x);  rx.data[10] = cos(x);  rx.data[14] = 0;
    rx.data[3] = 0;      rx.data[7] = 0;       rx.data[11] = 0;       rx.data[15] = 1;

    ry.data[0] = cos(y); ry.data[4] = 0;       ry.data[8]  = -sin(y); ry.data[12] = 0;
    ry.data[1] = 0;      ry.data[5] = 1;       ry.data[9]  = 0;       ry.data[13] = 0;
    ry.data[2] = sin(y); ry.data[6] = 0;       ry.data[10] = cos(y);  ry.data[14] = 0;
    ry.data[3] = 0;      ry.data[7] = 0;       ry.data[11] = 0;       ry.data[15] = 1;

    rz.data[0] = cos(z); rz.data[4] = -sin(z); rz.data[8]  = 0;       rz.data[12] = 0;
    rz.data[1] = sin(z); rz.data[5] = cos(z);  rz.data[9]  = 0;       rz.data[13] = 0;
    rz.data[2] = 0;      rz.data[6] = 0;       rz.data[10] = 1;       rz.data[14] = 0;
    rz.data[3] = 0;      rz.data[7] = 0;       rz.data[11] = 0;       rz.data[15] = 1;

    mul_two_mats(&rz, &ry, &tmp);
    mul_two_mats(&rx, &tmp, m);

    return;
}

static void make_transl_mat(matrix44_s *const m,
                            const float x, const float y, const float z)
{
    m->data[0] = 1;    m->data[4] = 0;    m->data[8]  = 0; m->data[12] = x;
    m->data[1] = 0;    m->data[5] = 1;    m->data[9]  = 0; m->data[13] = y;
    m->data[2] = 0;    m->data[6] = 0;    m->data[10] = 1; m->data[14] = z;
    m->data[3] = 0;    m->data[7] = 0;    m->data[11] = 0; m->data[15] = 1;

    return;
}

static void make_persp_mat(matrix44_s *const m,
                           const float fov, const float aspectRatio,
                           const float zNear, const float zFar)
{
    float tanHalfFOV = tan(fov / 2);
    float zRange = zNear - zFar;

    m->data[0] = 1.0f / (tanHalfFOV * aspectRatio); m->data[4] = 0;                 m->data[8] = 0;                         m->data[12] = 0;
    m->data[1] = 0;                                 m->data[5] = 1.0f / tanHalfFOV; m->data[9] = 0;                         m->data[13] = 0;
    m->data[2] = 0;                                 m->data[6] = 0;                 m->data[10] = (-zNear -zFar)/zRange;    m->data[14] = 2 * zFar * zNear / zRange;
    m->data[3] = 0;                                 m->data[7] = 0;                 m->data[11] = 1;                        m->data[15] = 0;

    return;
}

static void make_scaling_mat(matrix44_s *const m,
                             const float x, const float y, const float z)
{
    m->data[0] = x;    m->data[4] = 0;    m->data[8]  = 0;    m->data[12] = 0;
    m->data[1] = 0;    m->data[5] = y;    m->data[9]  = 0;    m->data[13] = 0;
    m->data[2] = 0;    m->data[6] = 0;    m->data[10] = z;    m->data[14] = 0;
    m->data[3] = 0;    m->data[7] = 0;    m->data[11] = 0;    m->data[15] = 1;

    return;
}

static void make_screen_space_mat(matrix44_s *const m,
                                  const float halfWidth, const float halfHeight)
{
    m->data[0] = halfWidth; m->data[4] = 0;             m->data[8]  = 0;    m->data[12] = halfWidth - 0.5f;
    m->data[1] = 0;         m->data[5] = -halfHeight;   m->data[9]  = 0;    m->data[13] = halfHeight - 0.5f;
    m->data[2] = 0;         m->data[6] = 0;             m->data[10] = 1;    m->data[14] = 0;
    m->data[3] = 0;         m->data[7] = 0;             m->data[11] = 0;    m->data[15] = 1;

    return;
}

void initialize_geometry(const unsigned renderWidth, const unsigned renderHeight)
{
    make_screen_space_mat(&SCREEN_SPACE_MAT, (renderWidth / 2.0f), (renderHeight / 2.0f));
    make_persp_mat(&PERSP_MAT, DEG_TO_RAD(60), (renderWidth / (float)renderHeight), NEAR_CLIP, FAR_CLIP);

    return;
}

static void tri_perspective_divide(struct shiet_polygon_triangle_s *const t)
{
    unsigned i;

    for (i = 0; i < 3; i++)
    {
        t->vertex[i].x /= t->vertex[i].w;
        t->vertex[i].y /= t->vertex[i].w;
        t->vertex[i].z /= t->vertex[i].w;
    }

    return;
}

unsigned transform_triangles(struct shiet_polygon_triangle_s *const triangles,
                             const unsigned numTriangles,
                             struct shiet_polygon_triangle_s *const transformedTriangles)
{
    static float rot = 0;
    unsigned i, numTransformedTriangles = 0;
    matrix44_s rotation, transl, worldSpace, clipSpace;

    /* Add some placeholder rotation, for visual interest.*/
    rot += 0.02;

    /* Create the world-space matrix from the translation and rotation matrices,
     * and pre-bake the perspective matrix into the world space matrix, producing
     * the clip-space matrix.*/
    make_transl_mat(&transl, 0, 0, 1.5);
    make_rot_mat(&rotation, 0, rot, 0);
    mul_two_mats(&transl, &rotation, &worldSpace);
    mul_two_mats(&PERSP_MAT, &worldSpace, &clipSpace);

    for (i = 0; i < numTriangles; i++)
    {
        int triIsVisible = 1;

        transformedTriangles[numTransformedTriangles] = triangles[i];

        /* Transform into clip-space.*/
        transform_vert(&transformedTriangles[numTransformedTriangles].vertex[0], &clipSpace);
        transform_vert(&transformedTriangles[numTransformedTriangles].vertex[1], &clipSpace);
        transform_vert(&transformedTriangles[numTransformedTriangles].vertex[2], &clipSpace);

        /* Something might happen in clip-space at some point, but not now.*/

        /* Transform into screen-space.*/
        transform_vert(&transformedTriangles[numTransformedTriangles].vertex[0], &SCREEN_SPACE_MAT);
        transform_vert(&transformedTriangles[numTransformedTriangles].vertex[1], &SCREEN_SPACE_MAT);
        transform_vert(&transformedTriangles[numTransformedTriangles].vertex[2], &SCREEN_SPACE_MAT);
        tri_perspective_divide(&transformedTriangles[numTransformedTriangles]);

        /* Back-face culling would go here.*/

        /* Depth-clipping would go here.*/

        /* If the triangle is visible, allow it to be drawn. If it's not, the
         * next triangle, indexed with k, will overwrite this one.*/
        if (triIsVisible)
        {
            numTransformedTriangles++;
        }
    }

    return numTransformedTriangles;
}
