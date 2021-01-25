/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A Direct3D 7 rasterizer surface for the Kelpo renderer.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#ifndef KELPO_RENDERER_SURFACE_DIRECT3D_7_SURFACE_DIRECT3D_7_H
#define KELPO_RENDERER_SURFACE_DIRECT3D_7_SURFACE_DIRECT3D_7_H

int kelpo_surface_direct3d_7__release_surface(void);

int kelpo_surface_direct3d_7__flip_surface(void);

int kelpo_surface_direct3d_7__create_surface(const unsigned width,
                                             const unsigned height,
                                             const unsigned bpp,
                                             const int vsyncEnabled,
                                             const unsigned deviceIdx);

#endif
