#ifndef SHIET_POLYGON_TRIANGLE_H
#define SHIET_POLYGON_TRIANGLE_H

#include <shiet_interface/polygon/texture.h>
#include <shiet_interface/polygon/vertex.h>

struct shiet_polygon_triangle_s
{
    struct shiet_polygon_vertex_s vertex[3];
    
    struct shiet_polygon_texture_s *texture;

    /* Note: Assume that each flag will be initialized to 0 by default.*/
    struct shiet_polygon_triangle_flags_s
    {
        unsigned wireframe : 1;          /* Render triangle with... 0 = no wireframe, 1 = wireframe                 */
        unsigned ignore : 1;             /* Triangle should... 0 = not be rendered, 1 = be rendered                 */
        unsigned twoSided : 1;           /* Backface culling... 0 = on, 1 = off                                     */
    } flags;
};

#endif
