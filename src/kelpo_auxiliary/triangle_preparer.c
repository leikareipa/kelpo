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
#include <kelpo_auxiliary/vector_3.h>
#include <kelpo_auxiliary/triangle_clipper.h>

static void transform_vert(struct kelpo_polygon_vertex_s *const v,
                           const struct kelpoa_matrix44_s *const m)
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

static void transform_normal(struct kelpo_polygon_vertex_s *const v,
                             const struct kelpoa_matrix44_s *const m)
{
    float x0 = ((m->elements[0] * v->nx) + (m->elements[4] * v->ny) + (m->elements[ 8] * v->nz));
    float y0 = ((m->elements[1] * v->nx) + (m->elements[5] * v->ny) + (m->elements[ 9] * v->nz));
    float z0 = ((m->elements[2] * v->nx) + (m->elements[6] * v->ny) + (m->elements[10] * v->nz));

    v->nx = x0;
    v->ny = y0;
    v->nz = z0;

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

void kelpoa_triprepr__duplicate_triangles(const struct kelpoa_generic_stack_s *const triangles,
                                          struct kelpoa_generic_stack_s *const duplicatedTriangles)
{
    kelpoa_generic_stack__grow(duplicatedTriangles, triangles->count);

    /* The triangle stack stores its data contiguously, so we can just memcpy()
     * it. Note that this only does a shallow copy.*/
    memcpy(duplicatedTriangles->data, triangles->data, (sizeof(struct kelpo_polygon_triangle_s) * triangles->count));

    duplicatedTriangles->count = triangles->count;

    return;
}

void kelpoa_triprepr__transform_triangles(struct kelpoa_generic_stack_s *const triangles,
                                          struct kelpoa_matrix44_s *const matrix)
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

void kelpoa_triprepr__transform_triangle_normals(struct kelpoa_generic_stack_s *const triangles,
                                                 struct kelpoa_matrix44_s *const matrix)
{
    unsigned i = 0;

    for (i = 0; i < triangles->count; i++)
    {
        struct kelpo_polygon_triangle_s *const triangle = &((struct kelpo_polygon_triangle_s*)triangles->data)[i];

        transform_normal(&triangle->vertex[0], matrix);
        transform_normal(&triangle->vertex[1], matrix);
        transform_normal(&triangle->vertex[2], matrix);
    }

    return;
}

void kelpoa_triprepr__rotate_triangles(struct kelpoa_generic_stack_s *const triangles,
                                       const float x,
                                       const float y,
                                       const float z)
{
    struct kelpoa_matrix44_s rotationMatrix;
    
    kelpoa_matrix44__make_rotation_matrix(&rotationMatrix, x, y, z);

    kelpoa_triprepr__transform_triangles(triangles, &rotationMatrix);
    kelpoa_triprepr__transform_triangle_normals(triangles, &rotationMatrix);

    return;
}

void kelpoa_triprepr__translate_triangles(struct kelpoa_generic_stack_s *const triangles,
                                          const float x,
                                          const float y,
                                          const float z)
{
    struct kelpoa_matrix44_s translationMatrix;
    
    kelpoa_matrix44__make_translation_matrix(&translationMatrix, x, y, z);

    kelpoa_triprepr__transform_triangles(triangles, &translationMatrix);

    return;
}

void kelpoa_triprepr__scale_triangles(struct kelpoa_generic_stack_s *const triangles,
                                      const float x,
                                      const float y,
                                      const float z)
{
    struct kelpoa_matrix44_s scalingMatrix;
    
    kelpoa_matrix44__make_scaling_matrix(&scalingMatrix, x, y, z);

    kelpoa_triprepr__transform_triangles(triangles, &scalingMatrix);

    return;
}

void kelpoa_triprepr__project_triangles_to_screen(const struct kelpoa_generic_stack_s *const triangles,
                                                  struct kelpoa_generic_stack_s *const screenSpaceTriangles,
                                                  const struct kelpoa_matrix44_s *const clipSpaceMatrix,
                                                  const struct kelpoa_matrix44_s *const screenSpaceMatrix,
                                                  const int backfaceCull)
{
    unsigned i = 0;

    for (i = 0; i < triangles->count; i++)
    {
        struct kelpo_polygon_triangle_s *const triangle = &((struct kelpo_polygon_triangle_s*)triangles->data)[i];

        if (backfaceCull)
        {
            struct kelpoa_vector3_s viewVector;
            struct kelpoa_vector3_s surfaceNormal;

            surfaceNormal.x = triangle->vertex[0].nx;
            surfaceNormal.y = triangle->vertex[0].ny;
            surfaceNormal.z = triangle->vertex[0].nz;

            viewVector.x = triangle->vertex[0].x;
            viewVector.y = triangle->vertex[0].y;
            viewVector.z = triangle->vertex[0].z;

            if (kelpoa_vector3__dot(&surfaceNormal, &viewVector) >= 0)
            {
                continue;
            }
        }

        transform_vert(&triangle->vertex[0], clipSpaceMatrix);
        transform_vert(&triangle->vertex[1], clipSpaceMatrix);
        transform_vert(&triangle->vertex[2], clipSpaceMatrix);

        /* Clip the triangle against the viewing frustum, and transform the clippings
         * into screen space.*/
        {
            unsigned k = 0;
            struct kelpo_polygon_triangle_s *clippedTris;
            const unsigned numClippedTris = kelpoa_triclipr__clip_triangle(triangle, &clippedTris);

            for (k = 0; k < numClippedTris; k++)
            {
                transform_vert(&clippedTris[k].vertex[0], screenSpaceMatrix);
                transform_vert(&clippedTris[k].vertex[1], screenSpaceMatrix);
                transform_vert(&clippedTris[k].vertex[2], screenSpaceMatrix);
                tri_perspective_divide(&clippedTris[k]);

                kelpoa_generic_stack__push_copy(screenSpaceTriangles, &clippedTris[k]);
            }
        }
    }

    return;
}
