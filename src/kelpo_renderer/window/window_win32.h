#ifndef KELPO_RENDERER_WINDOW_WINDOW_WIN32_H
#define KELPO_RENDERER_WINDOW_WINDOW_WIN32_H

#include <kelpo_interface/common/stdint.h>
#include <kelpo_interface/interface.h>
#include <kelpo_auxiliary/misc.h>

uint32_t kelpo_window__get_window_handle(void);

void kelpo_window__set_external_message_handler(kelpo_custom_window_message_handler_t *const messageHandler);

void kelpo_window__create_window(const unsigned width,
                                 const unsigned height,
                                 const char *const title,
                                 kelpo_custom_window_message_handler_t *const messageHandler);

void kelpo_window__get_window_size(void);

void kelpo_window__process_window_events(void);

int kelpo_window__is_window_open(void);

#endif
