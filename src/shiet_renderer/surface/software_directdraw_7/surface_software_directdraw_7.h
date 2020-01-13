/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * OpenGL render surface for the shiet renderer.
 * 
 */

#ifndef SHIET_SURFACE_SOFTWARE_DIRECTDRAW_7_H
#define SHIET_SURFACE_SOFTWARE_DIRECTDRAW_7_H

void shiet_surface_software_directdraw_7__release_surface(void);

void shiet_surface_software_directdraw_7__flip_surface(void);

void shiet_surface_software_directdraw_7__create_surface(const unsigned width,
                                                               const unsigned height);

#endif
