/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED common routines for Win32 displays
 *
 */

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <kelpo_renderer/window/win32/window_win32.h>
#include <kelpo_interface/error.h>

static char WINDOW_CLASS_NAME[] = "KelpoDisplay";
static char WINDOW_TITLE[64];
static HWND WINDOW_HANDLE = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static int DOES_WINDOW_EXIST = 0;
static HINSTANCE WINDOW_H_INSTANCE = 0;

/* A pointer to a function provided by the creator of this window. The function
 * will receive the window's messages.*/
static kelpo_custom_window_message_handler_t *WINDOW_OWNER_MESSAGE_HANDLER = 0;

/* A pointer to an externally-provided function that will receive the window's
 * messages. Generally, this function will be provided by the end-user of the
 * renderer.*/
static kelpo_custom_window_message_handler_t *EXTERNAL_MESSAGE_HANDLER = 0;

int kelpo_window__is_window_open(void)
{
    return DOES_WINDOW_EXIST;
}

static LRESULT CALLBACK window_message_handler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (EXTERNAL_MESSAGE_HANDLER &&
        EXTERNAL_MESSAGE_HANDLER(windowHandle, message, wParam, lParam))
    {
        return 0;
    }

    if (WINDOW_OWNER_MESSAGE_HANDLER &&
        WINDOW_OWNER_MESSAGE_HANDLER(windowHandle, message, wParam, lParam))
    {
        return 0;
    }

    switch (message)
    {
        case WM_CLOSE:
        {
            WINDOW_HANDLE = 0;
            kelpo_window__release_window();

            break;
        }

        default: return DefWindowProc(windowHandle, message, wParam, lParam);
    }
    
    return 0;
}

uint32_t kelpo_window__get_window_handle(void)
{
    return (uint32_t)WINDOW_HANDLE;
}

void kelpo_window__set_external_message_handler(kelpo_custom_window_message_handler_t *const messageHandler)
{
    EXTERNAL_MESSAGE_HANDLER = messageHandler;
    
    return;
}

void kelpo_window__release_window(void)
{
    if (WINDOW_HANDLE)
    {
        if (!DestroyWindow(WINDOW_HANDLE))
        {
            kelpo_error(KELPOERR_API_CALL_FAILED);
        }

        if (WINDOW_H_INSTANCE &&
            !UnregisterClass(WINDOW_CLASS_NAME, WINDOW_H_INSTANCE))
        {
            kelpo_error(KELPOERR_API_CALL_FAILED);
        }

        WINDOW_HANDLE = 0;
    }

    WINDOW_H_INSTANCE = 0;
    DOES_WINDOW_EXIST = 0;

    return;
}

void kelpo_window__create_window(const unsigned width,
                                 const unsigned height,
                                 const char *const title,
                                 kelpo_custom_window_message_handler_t *const messageHandler)
{
    WNDCLASSA wc;

    WINDOW_H_INSTANCE = GetModuleHandle(NULL);
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    WINDOW_OWNER_MESSAGE_HANDLER = messageHandler;

    assert((strlen(title) < KELPOA_NUM_ARRAY_ELEMENTS(WINDOW_TITLE)) && "The given window title is too long.");
    sprintf(WINDOW_TITLE, "%s", title);

    memset(&wc, 0, sizeof(wc));
    wc.style = 0;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.lpfnWndProc = window_message_handler;
    wc.hInstance = WINDOW_H_INSTANCE;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassA(&wc);

    WINDOW_HANDLE = CreateWindowEx(WS_EX_TOPMOST,
                                   WINDOW_CLASS_NAME,
                                   WINDOW_TITLE,
                                   WS_POPUP,
                                   0,
                                   0,
                                   WINDOW_WIDTH,
                                   WINDOW_HEIGHT,
                                   NULL,
                                   NULL,
                                   WINDOW_H_INSTANCE,
                                   NULL);

    if (!WINDOW_HANDLE)
    {
        /* TODO: Return false.*/
        return;
    }
    
    DOES_WINDOW_EXIST = 1;

    return;
}

void kelpo_window__process_window_messages(void)
{
    MSG m;

    InvalidateRect(WINDOW_HANDLE, NULL, FALSE);

    while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }

    return;
}
