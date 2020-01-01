/*
 * Tarpeeksi Hyvae Soft 2019
 * 
 * Software: shiet and its related functionality
 * 
 * Transforms triangles into screen space and makes them rotate around the origin.
 * 
 * USAGE:
 * 
 *   1. Call trirot_initialize_screen_geometry() with your screen (render) resolution.
 *      This has to be done only once in the program's lifetime, but prior to attempting 
 *      to have any triangles transformed.
 * 
 *   2. Call trirot_transform_and_rotate_triangles() with the triangles to be transformed,
 *      and a destination triangle buffer in which to place the transformed triangles (the
 *      operation is not in-place).
 * 
 * 
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
#include "transform_and_rotate_triangles.h"

struct matrix44_s
{
    float elements[4*4];
};

static const float NEAR_CLIP = 1;
static const float FAR_CLIP = 2000;

static struct matrix44_s SCREEN_SPACE_MAT;
static struct matrix44_s PERSP_MAT;

static void transform_vert(struct shiet_polygon_vertex_s *const v,
                           const struct matrix44_s *const m)
{
    float x0 = ((m->elements[0] * v->x) + (m->elements[4] * v->y) + (m->elements[ 8] * v->z) + (m->elements[12] * v->w));
    float y0 = ((m->elements[1] * v->x) + (m->elements[5] * v->y) + (m->elements[ 9] * v->z) + (m->elements[13] * v->w));
    float z0 = ((m->elements[2] * v->x) + (m->elements[6] * v->y) + (m->elements[10] * v->z) + (m->elements[14] * v->w));
    float w0 = ((m->elements[3] * v->x) + (m->elements[7] * v->y) + (m->elements[11] * v->z) + (m->elements[15] * v->w));

    v->x = x0;
    v->y = y0;
    v->z = z0;
    v->w = w0;

    return;
}

static void mul_two_mats(const struct matrix44_s *const m1,
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

static void make_rot_mat(struct matrix44_s *const m,
                         float x, float y, float z)
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

    mul_two_mats(&rz, &ry, &tmp);
    mul_two_mats(&rx, &tmp, m);

    return;
}

static void make_transl_mat(struct matrix44_s *const m,
                            const float x, const float y, const float z)
{
    m->elements[0] = 1;    m->elements[4] = 0;    m->elements[8]  = 0; m->elements[12] = x;
    m->elements[1] = 0;    m->elements[5] = 1;    m->elements[9]  = 0; m->elements[13] = y;
    m->elements[2] = 0;    m->elements[6] = 0;    m->elements[10] = 1; m->elements[14] = z;
    m->elements[3] = 0;    m->elements[7] = 0;    m->elements[11] = 0; m->elements[15] = 1;

    return;
}

static void make_persp_mat(struct matrix44_s *const m,
                           const float fov, const float aspectRatio,
                           const float zNear, const float zFar)
{
    float tanHalfFOV = tan(fov / 2);
    float zRange = zNear - zFar;

    m->elements[0] = 1.0f / (tanHalfFOV * aspectRatio); m->elements[4] = 0;                 m->elements[8] = 0;                         m->elements[12] = 0;
    m->elements[1] = 0;                                 m->elements[5] = 1.0f / tanHalfFOV; m->elements[9] = 0;                         m->elements[13] = 0;
    m->elements[2] = 0;                                 m->elements[6] = 0;                 m->elements[10] = (-zNear -zFar)/zRange;    m->elements[14] = 2 * zFar * zNear / zRange;
    m->elements[3] = 0;                                 m->elements[7] = 0;                 m->elements[11] = 1;                        m->elements[15] = 0;

    return;
}

static void make_scaling_mat(struct matrix44_s *const m,
                             const float x, const float y, const float z)
{
    m->elements[0] = x;    m->elements[4] = 0;    m->elements[8]  = 0;    m->elements[12] = 0;
    m->elements[1] = 0;    m->elements[5] = y;    m->elements[9]  = 0;    m->elements[13] = 0;
    m->elements[2] = 0;    m->elements[6] = 0;    m->elements[10] = z;    m->elements[14] = 0;
    m->elements[3] = 0;    m->elements[7] = 0;    m->elements[11] = 0;    m->elements[15] = 1;

    return;
}

static void make_screen_space_mat(struct matrix44_s *const m,
                                  const float halfWidth, const float halfHeight)
{
    m->elements[0] = halfWidth; m->elements[4] = 0;             m->elements[8]  = 0;    m->elements[12] = halfWidth - 0.5f;
    m->elements[1] = 0;         m->elements[5] = -halfHeight;   m->elements[9]  = 0;    m->elements[13] = halfHeight - 0.5f;
    m->elements[2] = 0;         m->elements[6] = 0;             m->elements[10] = 1;    m->elements[14] = 0;
    m->elements[3] = 0;         m->elements[7] = 0;             m->elements[11] = 0;    m->elements[15] = 1;

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

void trirot_initialize_screen_geometry(const unsigned renderWidth, const unsigned renderHeight)
{
    make_screen_space_mat(&SCREEN_SPACE_MAT, (renderWidth / 2.0f), (renderHeight / 2.0f));
    make_persp_mat(&PERSP_MAT, DEG_TO_RAD(60), (renderWidth / (float)renderHeight), NEAR_CLIP, FAR_CLIP);

    return;
}

unsigned trirot_transform_and_rotate_triangles(struct shiet_polygon_triangle_s *const triangles,
                                               const unsigned numTriangles,
                                               struct shiet_polygon_triangle_s *const dst,
                                               const float rotX, const float rotY, const float rotZ,
                                               const float cameraDistance)
{
    static float rot[3] = {0};
    unsigned i, numTransformedTriangles = 0;
    struct matrix44_s rotation, transl, worldSpace, clipSpace;

    rot[0] += rotX;
    rot[1] += rotY;
    rot[2] += rotZ;

    /* Create the world-space matrix from the translation and rotation matrices,
     * and pre-bake the perspective matrix into the world space matrix, producing
     * the clip-space matrix.*/
    make_transl_mat(&transl, 0, 0, cameraDistance);
    make_rot_mat(&rotation, rot[0], rot[1], rot[2]);
    mul_two_mats(&transl, &rotation, &worldSpace);
    mul_two_mats(&PERSP_MAT, &worldSpace, &clipSpace);

    for (i = 0; i < numTriangles; i++)
    {
        int triIsVisible = 1;

        dst[numTransformedTriangles] = triangles[i];

        /* Transform into clip-space.*/
        transform_vert(&dst[numTransformedTriangles].vertex[0], &clipSpace);
        transform_vert(&dst[numTransformedTriangles].vertex[1], &clipSpace);
        transform_vert(&dst[numTransformedTriangles].vertex[2], &clipSpace);

        /* Something might happen in clip-space at some point, but not now.*/

        /* Transform into screen-space.*/
        transform_vert(&dst[numTransformedTriangles].vertex[0], &SCREEN_SPACE_MAT);
        transform_vert(&dst[numTransformedTriangles].vertex[1], &SCREEN_SPACE_MAT);
        transform_vert(&dst[numTransformedTriangles].vertex[2], &SCREEN_SPACE_MAT);
        tri_perspective_divide(&dst[numTransformedTriangles]);

        /* Back-face culling would go here.*/

        /* Depth clipping would go here.*/

        /* If the triangle is visible, allow it to be drawn. If it's not, the
         * next triangle, indexed with k, will overwrite this one.*/
        if (triIsVisible)
        {
            numTransformedTriangles++;
        }
    }

    return numTransformedTriangles;
}
