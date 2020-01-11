/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * OpenGL render surface for the shiet renderer.
 * 
 */

#ifndef SHIET_SURFACE_OPENGL_H
#define SHIET_SURFACE_OPENGL_H

void shiet_surface_opengl_1_2_win32__release_surface(void);

void shiet_surface_opengl_1_2_win32__flip_surface(void);

void shiet_surface_opengl_1_2_win32__create_surface(const unsigned width,
                                                    const unsigned height,
                                                    const char *const windowTitle);

#endif
