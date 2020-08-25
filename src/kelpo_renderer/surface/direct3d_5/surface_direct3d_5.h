/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A Direct3D 5 rasterizer surface for the Kelpo renderer.
 * 
 * Note: The DirectX 5 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#ifndef KELPO_RENDERER_SURFACE_DIRECT3D_5_SURFACE_DIRECT3D_5_H
#define KELPO_RENDERER_SURFACE_DIRECT3D_5_SURFACE_DIRECT3D_5_H

void kelpo_surface_direct3d_5__release_surface(void);

void kelpo_surface_direct3d_5__flip_surface(void);

void kelpo_surface_direct3d_5__create_surface(const unsigned width,
                                              const unsigned height,
                                              const unsigned bpp,
                                              const int vsyncEnabled,
                                              const unsigned deviceIdx);

#endif
