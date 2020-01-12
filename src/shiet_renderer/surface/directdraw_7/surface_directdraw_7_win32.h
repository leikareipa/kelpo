/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Provides a Direct3D-compatible DirectDraw 7 surface for the shiet renderer.
 * 
 * Not intended to be used as a standalone surface, but as an intermediary for
 * other surfaces that require a DirectDraw 7 backend.
 * 
 * Note: The Direct3D 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of shiet.
 * 
 */

#ifndef SHIET_RENDERER_DIRECTDRAW_7_SURFACE_WIN32_H
#define SHIET_RENDERER_DIRECTDRAW_7_SURFACE_WIN32_H

#include <d3d.h>

void shiet_surface_directdraw_7_win32__release_surface(void);

/* Flips the front and back buffers.*/
void shiet_surface_directdraw_7_win32__flip_surface(void);

HRESULT shiet_surface_directdraw_7_win32__initialize_surface(const unsigned width,
                                                             const unsigned height,
                                                             const HWND windowHandle,
                                                             GUID *deviceGUID);

/* Creates and attaches a Z buffer of the given pixel format to the current back
 * buffer. The Z buffer will inherit the back buffer's dimensions. The D3D device
 * is obtained from shiet_surface_directdraw_7_win32__initialize_direct3d_7_interface ().*/
HRESULT shiet_surface_directdraw_7_win32__initialize_direct3d_7_zbuffer(LPDIRECT3DDEVICE7 d3dDevice,
                                                                        LPDDPIXELFORMAT pixelFormat);

/* Initializes a hardware Direct3D interface to the current DirectDraw surface,
 * and assigns the given pointers-to-pointers to point to the interface.*/
HRESULT shiet_surface_directdraw_7_win32__initialize_direct3d_7_interface(LPDIRECT3D7 *d3d,
                                                                          LPDIRECT3DDEVICE7 *d3dDevice);

#endif
