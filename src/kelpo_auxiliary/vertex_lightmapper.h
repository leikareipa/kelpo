/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 * Bakes light and shadow into triangle vertex colors.
 * 
 */

#ifndef KELPO_AUXILIARY_VERTEX_LIGHTMAPPER_H
#define KELPO_AUXILIARY_VERTEX_LIGHTMAPPER_H

#include <kelpo_auxiliary/vector_3d.h>

struct kelpoa_generic_stack_s;

struct kelpoa_vertlit_lightsource_s
{
    float x, y, z;
    float r, g, b;
    float intensity;
    float attenuation;
    float clip;
};

struct kelpoa_vertlit_ray_s
{
    struct kelpoa_vector3d_s pos;
    struct kelpoa_vector3d_s dir;
};

/* Modifies the given triangles' vertex colors based on their relation to the
 * given lights. Vertices that are directly visible to one or more light
 * source(s) will receive direct illumination, and vertices facing other,
 * directly-lit vertices will receive indirect illumination. The vertices are
 * expected to be in world space when calling this function.
 * 
 * WARNING: This function uses the vertices' W component for temporary storage,
 * and will reset its value to 1 on return.*/
void kelpoa_vertlit__bake(const struct kelpoa_generic_stack_s *const triangles,
                          const struct kelpoa_generic_stack_s *const lights);

#endif
