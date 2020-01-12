/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A Direct3D 7 rasterizer surface for the shiet renderer.
 * 
 * Note: The Direct3D 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of shiet.
 * 
 */

#ifndef SHIET_RENDERER_DIRECT3D_7_SURFACE_WIN32_H
#define SHIET_RENDERER_DIRECT3D_7_SURFACE_WIN32_H

void shiet_surface_direct3d_7_win32__release_surface(void);

void shiet_surface_direct3d_7_win32__flip_surface(void);

void shiet_surface_direct3d_7_win32__create_surface(const unsigned width,
                                                    const unsigned height);

#endif
