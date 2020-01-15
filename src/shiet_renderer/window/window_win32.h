#ifndef SHIET_RENDERER_WINDOW_WINDOW_WIN32_H
#define SHIET_RENDERER_WINDOW_WINDOW_WIN32_H

#include <WinDef.h>
#include <shiet_interface/common/stdint.h>

uint32_t shiet_window__get_window_handle(void);

void shiet_window__create_window(const unsigned width,
                                 const unsigned height,
                                 const char *const title,
                                 LRESULT (*customWindowProc)(HWND, UINT, WPARAM, LPARAM));

void shiet_window__get_window_size(void);

void shiet_window__process_window_events(void);

int shiet_window__is_window_open(void);

#endif
