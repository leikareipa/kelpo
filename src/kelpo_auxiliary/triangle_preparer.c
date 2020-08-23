/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 */

#include <string.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_auxiliary/triangle_preparer.h>
#include <kelpo_auxiliary/matrix_44.h>
#include <kelpo_auxiliary/triangle_clipper.h>

static void transform_vert(struct kelpo_polygon_vertex_s *const v,
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

static void tri_perspective_divide(struct kelpo_polygon_triangle_s *const t)
{
    unsigned i;

    for (i = 0; i < 3; i++)
    {
        t->vertex[i].x /= t->vertex[i].w;
        t->vertex[i].y /= t->vertex[i].w;
        t->vertex[i].z /= t->vertex[i].w;
        t->vertex[i].w = (1 / t->vertex[i].w);
    }

    return;
}

void triprepr_duplicate_triangles(const struct kelpo_generic_stack_s *const triangles,
                                  struct kelpo_generic_stack_s *const duplicatedTriangles)
{
    kelpo_generic_stack__grow(duplicatedTriangles, triangles->count);

    /* The triangle stack stores its data contiguously, so we can just memcpy()
     * it. Note that this only does a shallow copy.*/
    memcpy(duplicatedTriangles->data, triangles->data, (sizeof(struct kelpo_polygon_triangle_s) * triangles->count));

    duplicatedTriangles->count = triangles->count;

    return;
}

void triprepr_transform_triangles(struct kelpo_generic_stack_s *const triangles,
                                  struct matrix44_s *const matrix)
{
    unsigned i = 0;

    for (i = 0; i < triangles->count; i++)
    {
        struct kelpo_polygon_triangle_s *const triangle = &((struct kelpo_polygon_triangle_s*)triangles->data)[i];

        transform_vert(&triangle->vertex[0], matrix);
        transform_vert(&triangle->vertex[1], matrix);
        transform_vert(&triangle->vertex[2], matrix);
    }

    return;
}

void triprepr_rotate_triangles(struct kelpo_generic_stack_s *const triangles,
                               const float x,
                               const float y,
                               const float z)
{
    struct matrix44_s rotationMatrix;
    
    matrix44_make_rotation_matrix(&rotationMatrix, x, y, z);

    triprepr_transform_triangles(triangles, &rotationMatrix);

    return;
}

void triprepr_translate_triangles(struct kelpo_generic_stack_s *const triangles,
                                  const float x,
                                  const float y,
                                  const float z)
{
    struct matrix44_s translationMatrix;
    
    matrix44_make_translation_matrix(&translationMatrix, x, y, z);

    triprepr_transform_triangles(triangles, &translationMatrix);

    return;
}

void triprepr_scale_triangles(struct kelpo_generic_stack_s *const triangles,
                              const float x,
                              const float y,
                              const float z)
{
    struct matrix44_s scalingMatrix;
    
    matrix44_make_scaling_matrix(&scalingMatrix, x, y, z);

    triprepr_transform_triangles(triangles, &scalingMatrix);

    return;
}

void triprepr_project_triangles_to_screen(const struct kelpo_generic_stack_s *const triangles,
                                          struct kelpo_generic_stack_s *const screenSpaceTriangles,
                                          const struct matrix44_s *const clipSpaceMatrix,
                                          const struct matrix44_s *const screenSpaceMatrix)
{
    unsigned i = 0;

    for (i = 0; i < triangles->count; i++)
    {
        struct kelpo_polygon_triangle_s *const triangle = &((struct kelpo_polygon_triangle_s*)triangles->data)[i];

        transform_vert(&triangle->vertex[0], clipSpaceMatrix);
        transform_vert(&triangle->vertex[1], clipSpaceMatrix);
        transform_vert(&triangle->vertex[2], clipSpaceMatrix);

        /* Clip the triangle against the viewing frustum, and transform the clippings
         * into screen space.*/
        {
            unsigned k = 0;
            struct kelpo_polygon_triangle_s *clippedTris;
            const unsigned numClippedTris = triclip_clip_triangle(triangle, &clippedTris);

            for (k = 0; k < numClippedTris; k++)
            {
                transform_vert(&clippedTris[k].vertex[0], screenSpaceMatrix);
                transform_vert(&clippedTris[k].vertex[1], screenSpaceMatrix);
                transform_vert(&clippedTris[k].vertex[2], screenSpaceMatrix);
                tri_perspective_divide(&clippedTris[k]);

                kelpo_generic_stack__push_copy(screenSpaceTriangles, &clippedTris[k]);
            }
        }
    }

    return;
}
