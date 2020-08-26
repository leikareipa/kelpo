/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A C adaptation of Benny Bobaganoosh's triangle clipper.
 * 
 * Full attribution for Benny's original code:
 * {
 *     Copyright (c) 2014, Benny Bobaganoosh, https://github.com/BennyQBD/3DSoftwareRenderer
 *     All rights reserved.
 * 
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions are met:
 *
 *     1. Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *     2. Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *     ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * }
 * 
 */

#include <assert.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/polygon/vertex.h>
#include <kelpo_auxiliary/misc.h>
#include <kelpo_auxiliary/triangle_clipper.h>

/* A buffer to store the triangles we've clipped from a given input triangle.
 * An pointer to this buffer will be returned to the caller when they request
 * to have a triangle clipped.*/
static struct kelpo_polygon_triangle_s CLIPPED_TRIANGLES[KELPOA_TRICLIPR_MAX_NUM_CLIPPED_TRIANGLES];

/* Temporary buffers to hold vertex data during clipping.*/
static struct vert_buffer_s
{
    struct kelpo_polygon_vertex_s v[KELPOA_TRICLIPR_MAX_NUM_CLIPPED_VERTICES];
    unsigned idx;
} VERTEX_BUFFER_1,
  VERTEX_BUFFER_2;

static float vertex_component(const struct kelpo_polygon_vertex_s *const vertex,
                              const unsigned componentIdx)
{
    switch (componentIdx)
    {
        case 0: return vertex->x;
        case 1: return vertex->y;
        case 2: return vertex->z;
        default: assert(0 && "Unknown vertex component index.");
    }
}

static void clip_polygon_component(struct vert_buffer_s *vertsIn,
                                   struct vert_buffer_s *vertsOut,
                                   const unsigned componentIdx,
                                   const float componentFactor)
{
    unsigned i = 0;
    struct kelpo_polygon_vertex_s *prevVert = &vertsIn->v[vertsIn->idx - 1];
    float prevComponent = (vertex_component(prevVert, componentIdx) * componentFactor);
    int prevInside = (prevComponent <= prevVert->w);

    while (i < vertsIn->idx)
    {
        struct kelpo_polygon_vertex_s *curVert = &vertsIn->v[i++];

        float curComponent = (vertex_component(curVert, componentIdx) * componentFactor);
        int curInside = (curComponent <= curVert->w);

        if (curInside ^ prevInside)
        {
            float lerpStep = (prevVert->w - prevComponent) / ((prevVert->w - prevComponent) - (curVert->w - curComponent));

            vertsOut->v[vertsOut->idx].x = KELPOA_LERP(prevVert->x, curVert->x, lerpStep);
            vertsOut->v[vertsOut->idx].y = KELPOA_LERP(prevVert->y, curVert->y, lerpStep);
            vertsOut->v[vertsOut->idx].z = KELPOA_LERP(prevVert->z, curVert->z, lerpStep);
            vertsOut->v[vertsOut->idx].w = KELPOA_LERP(prevVert->w, curVert->w, lerpStep);

            vertsOut->v[vertsOut->idx].nx = KELPOA_LERP(prevVert->nx, curVert->nx, lerpStep);
            vertsOut->v[vertsOut->idx].ny = KELPOA_LERP(prevVert->ny, curVert->ny, lerpStep);
            vertsOut->v[vertsOut->idx].nz = KELPOA_LERP(prevVert->nz, curVert->nz, lerpStep);

            vertsOut->v[vertsOut->idx].r = KELPOA_LERP(prevVert->r, curVert->r, lerpStep);
            vertsOut->v[vertsOut->idx].g = KELPOA_LERP(prevVert->g, curVert->g, lerpStep);
            vertsOut->v[vertsOut->idx].b = KELPOA_LERP(prevVert->b, curVert->b, lerpStep);
            vertsOut->v[vertsOut->idx].a = KELPOA_LERP(prevVert->a, curVert->a, lerpStep);

            vertsOut->v[vertsOut->idx].u = KELPOA_LERP(prevVert->u, curVert->u, lerpStep);
            vertsOut->v[vertsOut->idx].v = KELPOA_LERP(prevVert->v, curVert->v, lerpStep);

            vertsOut->idx++;
        }

        if (curInside)
        {
            vertsOut->v[vertsOut->idx++] = *curVert;
        }

        prevVert = curVert;
        prevComponent = curComponent;
        prevInside = curInside;
    }

    return;
}

static int clip_polygon_axis(const int componentIdx)
{
    clip_polygon_component(&VERTEX_BUFFER_1, &VERTEX_BUFFER_2, componentIdx, 1);
    VERTEX_BUFFER_1.idx = 0;
    
    if (VERTEX_BUFFER_2.idx == 0)
    {
        return 0;
    }

    clip_polygon_component(&VERTEX_BUFFER_2, &VERTEX_BUFFER_1, componentIdx, -1);
    VERTEX_BUFFER_2.idx = 0;

    return (VERTEX_BUFFER_1.idx != 0);
}

unsigned kelpoa_triclipr__clip_triangle(const struct kelpo_polygon_triangle_s *const triangle,
                                        struct kelpo_polygon_triangle_s **dstClippedTriangles)
{
    unsigned numClippedTriangles = 0;
    
    *dstClippedTriangles = CLIPPED_TRIANGLES;

    VERTEX_BUFFER_1.idx = 0;
    VERTEX_BUFFER_2.idx = 0;

    VERTEX_BUFFER_1.v[VERTEX_BUFFER_1.idx++] = triangle->vertex[0];
    VERTEX_BUFFER_1.v[VERTEX_BUFFER_1.idx++] = triangle->vertex[1];
    VERTEX_BUFFER_1.v[VERTEX_BUFFER_1.idx++] = triangle->vertex[2];

    if (clip_polygon_axis(0) &&
        clip_polygon_axis(1) &&
        clip_polygon_axis(2))
    {
        unsigned i = 0;
        struct kelpo_polygon_vertex_s *initialVertex = &VERTEX_BUFFER_1.v[0];

        for (i = 1; i < (VERTEX_BUFFER_1.idx - 1); i++)
        {
            CLIPPED_TRIANGLES[numClippedTriangles].texture = triangle->texture;
            CLIPPED_TRIANGLES[numClippedTriangles].flags = triangle->flags;
            
            CLIPPED_TRIANGLES[numClippedTriangles].vertex[0] = *initialVertex;
            CLIPPED_TRIANGLES[numClippedTriangles].vertex[1] = VERTEX_BUFFER_1.v[i];
            CLIPPED_TRIANGLES[numClippedTriangles].vertex[2] = VERTEX_BUFFER_1.v[i+1];

            if (++numClippedTriangles >= KELPOA_TRICLIPR_MAX_NUM_CLIPPED_TRIANGLES)
            {
                break;
            }
        }
    }
    
    return numClippedTriangles;
}
