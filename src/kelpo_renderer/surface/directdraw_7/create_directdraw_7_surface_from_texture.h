/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo renderer
 * 
 * Creates a DirectDraw 7 surface out of a Kelpo texture's data.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of kelpo.
 * 
 */

#ifndef KELPO_RENDERER_SURFACE_DIRECTDRAW_7_CREATE_DIRECTDRAW_7_SURFACE_FROM_TEXTURE_H
#define KELPO_RENDERER_SURFACE_DIRECTDRAW_7_CREATE_DIRECTDRAW_7_SURFACE_FROM_TEXTURE_H

#include <d3d.h>

struct kelpo_polygon_texture_s;

/* Creates a new DirectDraw7 surface that is compatible with the given Direct3D7
 * device and contains the data from the given Kelpo texture. Returns a pointer
 * to the surface on success; NULL on failure.*/
LPDIRECTDRAWSURFACE7 kelpo_create_directdraw7_surface_from_texture(const struct kelpo_polygon_texture_s *const texture,
                                                                   LPDIRECT3DDEVICE7 d3dDevice);

#endif
