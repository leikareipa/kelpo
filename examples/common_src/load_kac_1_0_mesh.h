/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Loads a KAC 1.0 mesh into a shiet-compatible format.
 *
 */

#ifndef SHIET_EXAMPLES_COMMON_LOAD_KAC_1_0_MESH_H
#define SHIET_EXAMPLES_COMMON_LOAD_KAC_1_0_MESH_H

#include <stdint.h>

struct shiet_polygon_triangle_s;

/* Loads a triangle mesh - along with any associated textures - from the given
 * KAC 1.0 file. Takes in uninitialized (or NULL) pointers to pointers to the
 * destination triangle and texture buffers, which will be initialized by the
 * function call to point to memory holding the data read from the file (and
 * converted into the relevant shiet formats). Returns 1 on success; else 0.*/
uint32_t shiet_load_kac10_mesh(const char *const kacFilename,
                               struct shiet_polygon_triangle_s **dstTriangles, uint32_t *numTriangles,
                               struct shiet_polygon_texture_s **dstTextures, uint32_t *numTextures);

#endif
