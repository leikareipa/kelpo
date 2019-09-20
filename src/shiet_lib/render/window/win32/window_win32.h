#ifndef SHIET_WINDOW_WIN32_H_
#define SHIET_WINDOW_WIN32_H_

void shiet_window_win32__get_window_handle(void **handle);

void shiet_window_win32__create_window(const unsigned width, const unsigned height, const unsigned bpp, const char *const title);

void shiet_window_win32__get_window_size(void);

int shiet_window_win32__is_window_open(void);

#endif
