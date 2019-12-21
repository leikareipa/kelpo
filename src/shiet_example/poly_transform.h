/*
 * Tarpeeksi Hyvae Soft 2018 /
 * DOS C Compiler Benchmark
 *
 */

#ifndef GEOMETRY_H
#define GEOMETRY_H

struct shiet_polygon_triangle_s;

typedef struct
{
    float data[4*4];
}
matrix44_s;

void initialize_geometry(const unsigned renderWidth, const unsigned renderHeight);
unsigned transform_triangles(struct shiet_polygon_triangle_s *const triangles,
                             const unsigned numTriangles,
                             struct shiet_polygon_triangle_s *const transformedTriangles);

#endif
