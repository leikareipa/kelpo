#ifndef KELPO_INTERFACE_POLYGON_TRIANGLE_H
#define KELPO_INTERFACE_POLYGON_TRIANGLE_H

#include <kelpo_interface/polygon/texture.h>
#include <kelpo_interface/polygon/vertex.h>

struct kelpo_polygon_triangle_s
{
    struct kelpo_polygon_vertex_s vertex[3];
    
    struct kelpo_polygon_texture_s *texture;

    /* Note: Assume that each flag will be initialized to 0 by default.*/
    struct kelpo_polygon_triangle_flags_s
    {
        unsigned wireframe : 1;     /* Render triangle with... 0 = no wireframe, 1 = wireframe   */
        unsigned ignore : 1;        /* Triangle should... 0 = not be rendered, 1 = be rendered   */
        unsigned twoSided : 1;      /* Backface culling... 0 = on, 1 = off                       */
    } flags;
};

#endif
