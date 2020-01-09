/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: shiet renderer
 * 
 * Creates a DirectDraw7 surface out of a shiet texture's data.
 * 
 */

#ifndef SHIET_RENDERER_RASTERIZER_DIRECT3D_7_CREATE_DIRECT3D_TEXTURE_H
#define SHIET_RENDERER_RASTERIZER_DIRECT3D_7_CREATE_DIRECT3D_TEXTURE_H

#include <d3d.h>

struct shiet_polygon_texture_s;

/* Creates a new DirectDraw7 surface that is compatible with the given Direct3D7
 * device and contains the data from the given shiet texture. Returns a pointer
 * to the surface on success; NULL on failure.*/
LPDIRECTDRAWSURFACE7 shiet_create_directdraw7_surface_from_texture(const struct shiet_polygon_texture_s *const texture,
                                                                   LPDIRECT3DDEVICE7 d3dDevice);

#endif
