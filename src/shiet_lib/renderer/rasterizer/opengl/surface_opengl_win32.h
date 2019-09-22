#ifndef SHIET_SURFACE_OPENGL_H_
#define SHIET_SURFACE_OPENGL_H_

void shiet_surface_opengl_win32__release_surface(void);

void shiet_surface_opengl_win32__update_surface(void);

void shiet_surface_opengl_win32__create_surface(const unsigned width,
                                                const unsigned height,
                                                const char *const windowTitle);

#endif
