/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#include "default_window_message_handler.h"
#include <windows.h>

LRESULT default_window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                {
                    PostMessage(windowHandle, WM_CLOSE, 0, 0);
                    break;
                }
                default: break;
            }

            break;
        }

        /* Hide the mouse cursor.*/
        case WM_SETCURSOR:
        {
            SetCursor(NULL);
            return 1;
        }

        default: return 0;
    }

    return 1;
}
