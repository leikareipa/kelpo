/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Glide 3.x render surface for the Kelpo renderer.
 * 
 */

#ifndef KELPO_RENDERER_SURFACE_GLIDE_3_SURFACE_GLIDE_3_H
#define KELPO_RENDERER_SURFACE_GLIDE_3_SURFACE_GLIDE_3_H

void kelpo_surface_glide_3__release_surface(void);

void kelpo_surface_glide_3__flip_surface(void);

void kelpo_surface_glide_3__create_surface(const unsigned width,
                                           const unsigned height,
                                           const unsigned bpp,
                                           const int vsyncEnabled,
                                           const unsigned deviceIdx);

#endif
