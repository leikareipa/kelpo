/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Provides a double-buffered, optionally Direct3D-compatible DirectDraw 7
 * surface for the shiet renderer.
 * 
 * Not intended to be used as a standalone surface (e.g. this does not create
 * its own window), but as an intermediary for other surfaces that require a
 * DirectDraw 7 backend.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of shiet.
 * 
 * 
 * Usage for non-Direct3D:
 * 
 *   1. Call __initialize_surface(). This will initialize DirectDraw, set up
 *      surface pixel buffers, and assign the unit's local state.
 * 
 *   2. Call __lock_surface() to obtain a direct pointer to the pixel data of
 *      the surface's back buffer.
 * 
 *   3. Perform any pixel-writing using the pointer you obtained in (2).
 * 
 *   4. Call __unlock_surface() when done pixel-writing. Note that this will
 *      invalidate the pointer you obtained in (2).
 * 
 *   5. Call __flip_surface() to present the changes you made in (3).
 * 
 *   Repeat steps 2-5 for each frame of rendering. When ready to exit, call
 *   __release_surface().
 * 
 * 
 * Usage for Direct3D:
 * 
 *   1.  Call __initialize_surface(). This will initialize DirectDraw, set up
 *       surface pixel buffers, and assign the unit's local state.
 * 
 *   2.  Call  __initialize_direct3d_7_interface() to create a Direct3D render
 *       context for this DirectDraw surface.
 * 
 *   2b. (Optional.) Call __initialize_direct3d_7_zbuffer() to set up a Z buffer.
 * 
 *   3.  Use the Direct3D interface you obtained in (2) to render onto the
 *       surface.
 * 
 *   4.  Call __flip_surface() to present the changes you made in (3).
 * 
 *   Repeat steps 3-4 for each frame of rendering. When ready to exit, call
 *   __release_surface().
 * 
 */

#ifndef SHIET_RENDERER_DIRECTDRAW_7_SURFACE_WIN32_H
#define SHIET_RENDERER_DIRECTDRAW_7_SURFACE_WIN32_H

#include <d3d.h>

HRESULT shiet_surface_directdraw_7_win32__initialize_surface(const unsigned width,
                                                             const unsigned height,
                                                             const HWND windowHandle,
                                                             GUID directDrawDeviceGUID);

void shiet_surface_directdraw_7_win32__release_surface(void);

void shiet_surface_directdraw_7_win32__flip_surface(void);

/* TODO.
void shiet_surface_directdraw_7_win32__resize_surface(void);*/

/* Locks the surface's back buffer, which allows direct pixel manipulation to
 * be performed. Takes in a pointer to an allocated surface descriptor, whose
 * fields will be initialized by the call to match the properties of the back
 * buffer (a pointer to the buffer's raw pixel data will be in .lpSurface).
 * Returns 1 if the call succeeds; 0 otherwise.*/
int shiet_surface_directdraw_7_win32__lock_surface(LPDDSURFACEDESC2 surfaceDesc);

/* Unlocks the surface's back buffer, which allows DirectDraw to operate on it.
 * No direct modification of the surface's pixel data is allowed while unlocked.
 * Returns 1 if the call succeeds; 0 otherwise.*/
int shiet_surface_directdraw_7_win32__unlock_surface(void);

/* Initializes a hardware Direct3D interface to the current DirectDraw surface,
 * and assigns the given pointers-to-pointers to point to the interface.*/
HRESULT shiet_surface_directdraw_7_win32__initialize_direct3d_7_interface(LPDIRECT3D7 *d3d,
                                                                          LPDIRECT3DDEVICE7 *d3dDevice);

/* Creates and attaches a Z buffer of the given pixel format to the current back
 * buffer. The Z buffer will inherit the back buffer's dimensions. The Direct3D
 * device is obtained from __initialize_direct3d_7_interface().*/
HRESULT shiet_surface_directdraw_7_win32__initialize_direct3d_7_zbuffer(LPDIRECT3DDEVICE7 d3dDevice,
                                                                        LPDDPIXELFORMAT pixelFormat);

#endif
