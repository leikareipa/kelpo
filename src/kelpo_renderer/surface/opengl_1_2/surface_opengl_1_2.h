/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * OpenGL render surface for the Kelpo renderer.
 * 
 */

#ifndef KELPO_RENDERER_SURFACE_OPENGL_1_2_SURFACE_OPENGL_1_2_H
#define KELPO_RENDERER_SURFACE_OPENGL_1_2_SURFACE_OPENGL_1_2_H

void kelpo_surface_opengl_1_2__release_surface(void);

void kelpo_surface_opengl_1_2__flip_surface(void);

void kelpo_surface_opengl_1_2__create_surface(const unsigned width,
                                              const unsigned height,
                                              const unsigned bpp,
                                              const int vsyncEnabled,
                                              const unsigned deviceIdx);

#endif
