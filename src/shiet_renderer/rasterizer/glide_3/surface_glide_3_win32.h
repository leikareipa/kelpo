/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x render surface for the shiet renderer.
 * 
 */

#ifndef SHIET_SURFACE_GLIDE3_H
#define SHIET_SURFACE_GLIDE3_H

void shiet_surface_glide_3_win32__release_surface(void);

void shiet_surface_glide_3_win32__flip_surface(void);

void shiet_surface_glide_3_win32__create_surface(const unsigned width,
                                                 const unsigned height,
                                                 const char *const windowTitle);

#endif
