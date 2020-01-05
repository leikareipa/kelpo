/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Loads a KAC 1.0 mesh into a shiet-compatible format.
 *
 */

#ifndef SHIET_EXAMPLES_COMMON_LOAD_KAC_1_0_MESH_H
#define SHIET_EXAMPLES_COMMON_LOAD_KAC_1_0_MESH_H

#include <shiet/common/stdint.h>

struct shiet_polygon_triangle_stack_s;
struct shiet_polygon_triangle_s;

/* Loads a triangle mesh - along with any associated textures - from the given
 * KAC 1.0 file. Triangles will be placed in the given stack. For textures, takes
 * in an uninitialized (or NULL) pointer-to-pointer that will be initialized by
 * the function to point to memory allocated to hold the loaded texture data.
 * Returns 1 on success; else 0.*/
int shiet_load_kac10_mesh(const char *const kacFilename,
                          struct shiet_polygon_triangle_stack_s *dstTriangles,
                          struct shiet_polygon_texture_s **dstTextures,
                          uint32_t *numTextures);

#endif
