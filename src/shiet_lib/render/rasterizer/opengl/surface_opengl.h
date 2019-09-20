#ifndef SHIET_SURFACE_OPENGL_H_
#define SHIET_SURFACE_OPENGL_H_

void shiet_surface_opengl__release_surface(void);

void shiet_surface_opengl__update_surface(void);

void shiet_surface_opengl__create_surface(const unsigned width,
                                          const unsigned height,
                                          const unsigned bpp,
                                          const char *const windowTitle);

#endif
