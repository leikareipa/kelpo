/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A C adaptation of Benny Bobaganoosh's triangle clipper (https://github.com/BennyQBD/3DSoftwareRenderer).
 * 
 */

#ifndef KELPO_AUXILIARY_TRIANGLE_CLIPPER_H
#define KELPO_AUXILIARY_TRIANGLE_CLIPPER_H

/* The maximum number of triangles and vertices we can clip a source triangle into.
 * These will be used to size the working buffers on the stack, so better conservative
 * than greedy values.*/
#define KELPOA_TRICLIP_MAX_NUM_CLIPPED_VERTICES 9
#define KELPOA_TRICLIP_MAX_NUM_CLIPPED_TRIANGLES (KELPOA_TRICLIP_MAX_NUM_CLIPPED_VERTICES - 2)

struct kelpo_polygon_triangle_s;

/* Clips the given triangle (whose vertices are in [-1,1] clip space) against the
 * view frustum, The 'dstClippedTriangles' pointer will be assigned the address of a
 * memory buffer holding the clipped triangles - the original triangle will not be
 * modified. The clipping buffer to which a pointer is given is shared between all
 * calls to this function, so its contents can be trusted to remain valid only until
 * the function is called again. Returns the number of triangles held in the memory
 * buffer at the time the function exited; or 0 if the triangle could not be clipped.*/
unsigned kelpoa_triclip__clip_triangle(const struct kelpo_polygon_triangle_s *const triangle,
                                       struct kelpo_polygon_triangle_s **dstClippedTriangles);

#endif
