/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 * Bakes light and shadow into triangle vertex colors.
 * 
 */

#include <math.h>
#include <float.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <kelpo_auxiliary/vertex_lightmapper.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_auxiliary/vector_3d.h>
#include <kelpo_interface/stdint.h>

static const double EPSILON = 0.00001;
static const double M_PI = 3.14159265358979323846;

/* Returns the distance from the given ray's origin to its intersection with
 * the given triangle; or DBL_MAX if the ray doesn't intersect the triangle.
 * Adapted from code provided in Moller & Trumbore 1997: "Fast, minimum storage
 * ray/triangle intersection".*/
static double intersect_ray_to_triangle(const struct kelpoa_vertlit_ray_s *const ray,
                                        const struct kelpo_polygon_triangle_s *const triangle)
{
    const double noHit = DBL_MAX;
    double det = 0;
    double invD = 0;
    double u = 0;
    double v = 0;
    double distance = 0;

    struct kelpoa_vector3d_s tv;
    struct kelpoa_vector3d_s qv;

    const struct kelpoa_vector3d_s e1 = kelpoa_vector3d((triangle->vertex[1].x - triangle->vertex[0].x),
                                                        (triangle->vertex[1].y - triangle->vertex[0].y),
                                                        (triangle->vertex[1].z - triangle->vertex[0].z));

    const struct kelpoa_vector3d_s e2 = kelpoa_vector3d((triangle->vertex[2].x - triangle->vertex[0].x),
                                                        (triangle->vertex[2].y - triangle->vertex[0].y),
                                                        (triangle->vertex[2].z - triangle->vertex[0].z));

    const struct kelpoa_vector3d_s pv = kelpoa_vector3d__cross(&ray->dir, &e2);

    det = kelpoa_vector3d__dot(&e1, &pv);
    if ((det > -EPSILON) && (det < EPSILON)) return noHit;

    invD = (1.0 / det);
    tv = kelpoa_vector3d((ray->pos.x - triangle->vertex[0].x),
                        (ray->pos.y - triangle->vertex[0].y),
                        (ray->pos.z - triangle->vertex[0].z));
    u = (kelpoa_vector3d__dot(&tv, &pv) * invD);
    if ((u < 0) || (u > 1)) return noHit;

    qv = kelpoa_vector3d__cross(&tv, &e1);
    v = (kelpoa_vector3d__dot(&ray->dir, &qv) * invD);
    if ((v < 0) || ((u + v) > 1)) return noHit;

    distance = (kelpoa_vector3d__dot(&e2, &qv) * invD);
    if (distance <= 0) return noHit;

    return distance;
}

/* Returns the distance from the given ray's origin to the closest triangle
 * intersected by the ray; or DBL_MAX if the ray intersects no triangle.*/
static double closest_ray_intersection(const struct kelpoa_vertlit_ray_s *const ray,
                                       const struct kelpoa_generic_stack_s *const triangles)
{
    uint32_t i = 0;
    double closestDistance = DBL_MAX;
    const struct kelpo_polygon_triangle_s *triangle = NULL;

    for ((i = 0, triangle = triangles->data); i < triangles->count; (i++, triangle++))
    {
        const double distance = intersect_ray_to_triangle(ray, triangle);

        /* To avoid "self-intersection" at shared vertices (i.e. immediately
         * intersecting another triangle at the point of the shared vertex), we
         * require valid intersection distances to be slightly non-zero.*/
        if ((distance > EPSILON) &&
            (distance < closestDistance))
        {
            closestDistance = distance;
        }
    }

    return closestDistance;
}

/* Returns 1 if the position x1,y1,z1 is visible from x0,y0,z0 in the given
 * scene of triangles; 0 otherwise. A point is considered to be visible from
 * another if there is no occluding geometry between the points, i.e. if the
 * distance from point A to the closest intersected triangle in the scene is
 * greater than the distance from point A to point B.*/
static int point_sees_point(const double x0,
                            const double y0,
                            const double z0,
                            const double x1,
                            const double y1,
                            const double z1,
                            const struct kelpoa_generic_stack_s *const triangles)
{
    const double pointDistance = sqrt(((x0 - x1) * (x0 - x1)) +
                                      ((y0 - y1) * (y0 - y1)) +
                                      ((z0 - z1) * (z0 - z1)));

    struct kelpoa_vertlit_ray_s ray;

    ray.pos = kelpoa_vector3d(x0, y0, z0);
    ray.dir = kelpoa_vector3d((x1 - x0), (y1 - y0), (z1 - z0));
    kelpoa_vector3d__normalize(&ray.dir);

    /* Note: If the target XYZ coordinates are those of a triangle's vertex,
     * the closest intersection might be with that triangle - i.e. the
     * intersected distance might be 99.999...% equal to the distance between
     * the two points. So we use epsilon to try and counter any doubleing-point
     * inaccuracy in that case.*/
    return ((closest_ray_intersection(&ray, triangles) - pointDistance) > EPSILON);
}

/* Returns a value in the range [0,1] representing the amount by which a quantity
 * of light would be attenuated when arriving to point 0 from point 1 given also
 * the surface normal n associated with point 0. A custom distance attenuation
 * factor can also be given; or set to 1 for the default value.*/
static double surface_scatter_attenuation(const double x0,
                                          const double y0,
                                          const double z0,
                                          const double xn,
                                          const double yn,
                                          const double zn,
                                          const double x1,
                                          const double y1,
                                          const double z1,
                                          const double distanceAttenuationFactor)
{
    double incidenceMul = 0;
    double distanceMul = 0;
    double brdf = 0;
    double pdf = 0;
    const double surfaceAlbedo = 0.8;
    const double pointDistance = sqrt(((x0 - x1) * (x0 - x1)) +
                                      ((y0 - y1) * (y0 - y1)) +
                                      ((z0 - z1) * (z0 - z1)));

    struct kelpoa_vector3d_s surfaceNormal = kelpoa_vector3d(xn, yn, zn);
    struct kelpoa_vector3d_s outDirection = kelpoa_vector3d((x1 - x0), (y1 - y0), (z1 - z0));

    kelpoa_vector3d__normalize(&outDirection);

    brdf = kelpoa_vector3d__dot(&surfaceNormal, &outDirection) * (surfaceAlbedo / M_PI);
    pdf = (1 / (2 * M_PI));
    
    incidenceMul = (brdf / pdf);
    if (incidenceMul < 0) incidenceMul = 0;
    if (incidenceMul > 1) incidenceMul = 1;

    distanceMul = (1 / (1 + (pointDistance * distanceAttenuationFactor)));
    if (distanceMul < 0) distanceMul = 0;
    if (distanceMul > 1) distanceMul = 1;

    return (incidenceMul * distanceMul);
}

/* TODO: Use a BVH tree to accelerate the ray-triangle intersection tests.*/
void kelpoa_vertlit__bake(const struct kelpoa_generic_stack_s *const triangles,
                          const struct kelpoa_generic_stack_s *const lights)
{
    uint32_t i = 0;
    struct kelpo_polygon_triangle_s *triangle = NULL;

    /* We'll use vertices' W component as temporary storage for lighting data.*/
    for ((i = 0, triangle = triangles->data); i < triangles->count; (i++, triangle++))
    {
        uint32_t v = 0;

        for (v = 0; v < 3; v++)
        {
            triangle->vertex[v].w = 0;
        }
    }

    /* To each vertex in the scene, trace a ray from each light source, and
     * shade the vertex by the brightest of the light sources. From each vertex
     * thus lit, trace a ray individually to all other vertices in the scene to
     * distribute the light to them also (think of this as the first indirect
     * light bounce).
     * 
     * Note that we (temporarily) store a vertex's shade in its W component.
     * This is done to save on having to create an extra array of shade values.*/ 
    for ((i = 0, triangle = triangles->data); i < triangles->count; (i++, triangle++))
    {
        uint32_t v = 0;
        struct kelpo_polygon_vertex_s *vertex = NULL;

        printf("\rBaking... %d%%", (int)((i / (double)triangles->count) * 100));

        for ((v = 0, vertex = &triangle->vertex[0]); v < 3; (v++, vertex++))
        {
            uint32_t l = 0;
            const struct kelpoa_vertlit_lightsource_s *light = NULL;

            for ((l = 0, light = lights->data); l < lights->count; (l++, light++))
            {
                /* We only need to consider lights that this triangle isn't
                 * facing away from.*/
                {
                    struct kelpoa_vector3d_s n1 = kelpoa_vector3d((vertex->x - light->x),
                                                                  (vertex->y - light->y),
                                                                  (vertex->z - light->z));
                    const struct kelpoa_vector3d_s n2 = kelpoa_vector3d(triangle->vertex[0].nx,
                                                                        triangle->vertex[0].ny,
                                                                        triangle->vertex[0].nz);

                    kelpoa_vector3d__normalize(&n1);

                    if (kelpoa_vector3d__dot(&n1, &n2) >= 0)
                    {
                        continue;
                    }
                }

                /* Calculate the amount of light falling on this vertex from
                 * this light source.*/
                if (point_sees_point((vertex->x + (vertex->nx * EPSILON)),
                                     (vertex->y + (vertex->ny * EPSILON)),
                                     (vertex->z + (vertex->nz * EPSILON)),
                                     light->x,
                                     light->y,
                                     light->z,
                                     triangles))
                {
                    double infall = light->intensity *
                                    surface_scatter_attenuation(vertex->x, vertex->y, vertex->z,
                                                                vertex->nx, vertex->ny, vertex->nz,
                                                                light->x, light->y, light->z, light->attenuation);

                    vertex->w = ((infall > vertex->w)? infall : vertex->w);
                }
            }

            /* Distribute the vertex's light to the other vertices in the scene.
             * Note that this will have no effect on the shade of vertices that
             * have already received a brighter shade from some other vertex;
             * i.e. the light doesn't accumulate.*/
            if (vertex->w > 0)
            {
                uint32_t i2 = 0;
                struct kelpo_polygon_triangle_s *dstTriangle = NULL;

                for ((i2 = 0, dstTriangle = triangles->data); i2 < triangles->count; (i2++, dstTriangle++))
                {
                    uint32_t v2 = 0;
                    struct kelpo_polygon_vertex_s *dstVertex = NULL;

                    if (dstTriangle == triangle)
                    {
                        continue;
                    }

                    /* We only need to consider vertices that point toward us.*/
                    {
                        const struct kelpoa_vector3d_s n1 = kelpoa_vector3d(vertex->nx, vertex->ny, vertex->nz);
                        const struct kelpoa_vector3d_s n2 = kelpoa_vector3d(dstTriangle->vertex[0].nx,
                                                                            dstTriangle->vertex[0].ny,
                                                                            dstTriangle->vertex[0].nz);

                        if (kelpoa_vector3d__dot(&n1, &n2) >= 0)
                        {
                            continue;
                        }
                    }

                    for ((v2 = 0, dstVertex = &dstTriangle->vertex[0]); v2 < 3; (v2++, dstVertex++))
                    {
                        /* To make indirect lighting more visually pronounced,
                         * we'll boost it by this multiplier.*/
                        const float lightMultiplier = 50;

                        if (dstVertex->w > (vertex->w * lightMultiplier))
                        {
                            continue;
                        }

                        /* Calculate the amount of light falling on this vertex from
                         * the source vertex.*/
                        if (point_sees_point((dstVertex->x + (dstVertex->nx * EPSILON)),
                                             (dstVertex->y + (dstVertex->ny * EPSILON)),
                                             (dstVertex->z + (dstVertex->nz * EPSILON)),
                                             (vertex->x + (vertex->nx * EPSILON)),
                                             (vertex->y + (vertex->ny * EPSILON)),
                                             (vertex->z + (vertex->nz * EPSILON)),
                                             triangles))
                        {
                            const double outLight = lightMultiplier *
                                                    vertex->w *
                                                    surface_scatter_attenuation(vertex->x, vertex->y, vertex->z,
                                                                                vertex->nx, vertex->ny, vertex->nz,
                                                                                dstVertex->x, dstVertex->y, dstVertex->z, 1);

                            dstVertex->w = ((dstVertex->w < outLight)? outLight : dstVertex->w);
                        }
                    }
                }
            }
        }
    }

    /* Apply each vertex's shade to its colors.*/
    for ((i = 0, triangle = triangles->data); i < triangles->count; (i++, triangle++))
    {
        uint32_t v = 0;
        struct kelpo_polygon_vertex_s *vertex = NULL;

        for ((v = 0, vertex = &triangle->vertex[0]); v < 3; (v++, vertex++))
        {
            unsigned r = (vertex->r * vertex->w);
            unsigned g = (vertex->g * vertex->w);
            unsigned b = (vertex->b * vertex->w);

            vertex->r = ((r < 255)? r : 255);
            vertex->g = ((g < 255)? g : 255);
            vertex->b = ((b < 255)? b : 255);

            /* Since we're done with lighting this vertex, i.e. with temporarily
             * storing shading data in its W component, we can reset the W
             * component back to its real value.*/
            vertex->w = 1;
        }
    }

    return;
}
