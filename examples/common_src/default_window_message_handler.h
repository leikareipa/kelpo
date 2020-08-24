/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Provides a default window message handler for Kelpo render examples.
 * 
 * 
 * Usage:
 * 
 *   1. Set up a kelpo_custom_window_message_handler_t function to receive
 *      messages from Kelpo's window.
 * 
 *   2. From the custom message handler, call default_window_message_handler()
 *      with the message's arguments. This function will evoke a default
 *      response, if any - for example, by closing the window if the user
 *      pressed the Escape key.
 * 
 */

#ifndef KELPO_EXAMPLES_COMMON_SRC_DEFAULT_WINDOW_MESSAGE_HANDLER_H
#define KELPO_EXAMPLES_COMMON_SRC_DEFAULT_WINDOW_MESSAGE_HANDLER_H

#include <windef.h>

/* Default handling of window messages from the Kelpo render example window.
 * Returns 1 if the message was handled; 0 otherwise. Expected to have the
 * signature of kelpo_custom_window_message_handler_t. */
LRESULT default_window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

#endif
