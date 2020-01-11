#ifndef SHIET_WINDOW_WIN32_H_
#define SHIET_WINDOW_WIN32_H_

#include <WinDef.h>
#include <shiet_interface/common/stdint.h>

uint32_t shiet_window_win32__get_window_handle(void);

void shiet_window_win32__create_window(const unsigned width, const unsigned height, const char *const title,
                                       LRESULT (*customWindowProc)(HWND, UINT, WPARAM, LPARAM));

void shiet_window_win32__get_window_size(void);

void shiet_window_win32__process_window_events(void);

int shiet_window_win32__is_window_open(void);

#endif
