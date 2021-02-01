/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * OpenGL render surface for the Kelpo renderer.
 * 
 */

#ifndef KELPO_RENDERER_SURFACE_opengl_1_1_SURFACE_opengl_1_1_H
#define KELPO_RENDERER_SURFACE_opengl_1_1_SURFACE_opengl_1_1_H

#include <windef.h>

LRESULT kelpo_surface_opengl_1_1__window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

int kelpo_surface_opengl_1_1__release_surface(void);

int kelpo_surface_opengl_1_1__flip_surface(void);

int kelpo_surface_opengl_1_1__create_surface(const unsigned width,
                                             const unsigned height,
                                             const unsigned bpp,
                                             const int vsyncEnabled,
                                             const unsigned deviceIdx);

#endif
