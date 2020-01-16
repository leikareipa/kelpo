/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Loads a KAC 1.0 mesh into a kelpo-compatible format.
 *
 */

#ifndef KELPO_EXAMPLES_COMMON_SRC_LOAD_KAC_1_0_MESH_H
#define KELPO_EXAMPLES_COMMON_SRC_LOAD_KAC_1_0_MESH_H

#include <kelpo_interface/common/stdint.h>

struct kelpo_generic_stack_s;
struct kelpo_polygon_triangle_s;

/* Loads a triangle mesh - along with any associated textures - from the given
 * KAC 1.0 file. Triangles will be placed in the given stack. For textures, takes
 * in an uninitialized (or NULL) pointer-to-pointer that will be initialized by
 * the function to point to memory allocated to hold the loaded texture data.
 * Returns 1 on success; else 0.
 * 
 * If the call returns 0, the destination texture buffer may or may not have been
 * allocated memory for by the call; but the call will not free the buffer in any
 * case. It's best to initialize the pointer to NULL prior to passing it into the
 * function, and if the call returns an error, check the pointer's value to see
 * whether it should be deallocated.
 */
int kelpo_load_kac10_mesh(const char *const kacFilename,
                          struct kelpo_generic_stack_s *dstTriangles,
                          struct kelpo_polygon_texture_s **dstTextures,
                          uint32_t *numTextures);

#endif
