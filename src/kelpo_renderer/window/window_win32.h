#ifndef KELPO_RENDERER_WINDOW_WINDOW_WIN32_H
#define KELPO_RENDERER_WINDOW_WINDOW_WIN32_H

#include <WinDef.h>
#include <kelpo_interface/common/stdint.h>

uint32_t kelpo_window__get_window_handle(void);

void kelpo_window__create_window(const unsigned width,
                                 const unsigned height,
                                 const char *const title,
                                 LRESULT (*customWindowProc)(HWND, UINT, WPARAM, LPARAM));

void kelpo_window__get_window_size(void);

void kelpo_window__process_window_events(void);

int kelpo_window__is_window_open(void);

#endif
