/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A Direct3D 6 rasterizer surface for the Kelpo renderer.
 * 
 * Note: The DirectX 6 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#ifndef KELPO_RENDERER_SURFACE_DIRECT3D_6_SURFACE_DIRECT3D_6_H
#define KELPO_RENDERER_SURFACE_DIRECT3D_6_SURFACE_DIRECT3D_6_H

int kelpo_surface_direct3d_6__release_surface(void);

int kelpo_surface_direct3d_6__flip_surface(void);

int kelpo_surface_direct3d_6__create_surface(const unsigned width,
                                             const unsigned height,
                                             const unsigned bpp,
                                             const int vsyncEnabled,
                                             const unsigned deviceIdx);

#endif
