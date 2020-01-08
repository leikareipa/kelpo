/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED common routines for Win32 displays
 *
 */

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <shiet_lib/renderer/window/win32/window_win32.h>
#include <shiet_interface/common/globals.h>

static char WINDOW_CLASS_NAME[] = "ShietDisplay";
static char WINDOW_TITLE[64];
static HWND WINDOW_HANDLE = 0;
static unsigned WINDOW_WIDTH = 0;
static unsigned WINDOW_HEIGHT = 0;
static int WINDOW_ACTIVE = 0;

LRESULT (*CUSTOM_WINDOW_PROC)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int shiet_window_win32__is_window_open(void)
{
    return (WINDOW_HANDLE != NULL);
}

static LRESULT CALLBACK window_message_handler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!CUSTOM_WINDOW_PROC ||
        !CUSTOM_WINDOW_PROC(hWnd, message, wParam, lParam))
    {
        switch (message)
        {
            case WM_ACTIVATE:
            {
                WINDOW_ACTIVE = !HIWORD(wParam);
                
                break;
            }

            case WM_DESTROY:
            {
                PostQuitMessage(0);
                WINDOW_HANDLE = NULL;

                break;
            }

            default: return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return 0;
}

uint32_t shiet_window_win32__get_window_handle(void)
{
    return (uint32_t)WINDOW_HANDLE;
}

void shiet_window_win32__create_window(const unsigned width, const unsigned height, const char *const title,
                                       LRESULT (*customWindowProc)(HWND, UINT, WPARAM, LPARAM))
{
    const DWORD dwStyle = (WS_CAPTION | WS_BORDER | WS_SYSMENU);
    const HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSA wc;
    RECT rc;

    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    CUSTOM_WINDOW_PROC = customWindowProc;

    assert((strlen(title) < NUM_ARRAY_ELEMENTS(WINDOW_TITLE)) && "The given window title is too long.");
    sprintf(WINDOW_TITLE, "%s", title);

    memset(&wc, 0, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.lpfnWndProc = window_message_handler;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassA(&wc);

    rc.left = 0;
    rc.top = 0;
    rc.right = WINDOW_WIDTH;
    rc.bottom = WINDOW_HEIGHT;
    AdjustWindowRect(&rc, dwStyle, FALSE);

    WINDOW_HANDLE = CreateWindowA(WINDOW_CLASS_NAME,
                                  WINDOW_TITLE,
                                  dwStyle,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  rc.right - rc.left,
                                  rc.bottom - rc.top,
                                  NULL,
                                  NULL,
                                  hInstance,
                                  NULL);

    assert((WINDOW_HANDLE != NULL) &&
           "Failed to create a window for the program. Can't continue.");

    return;
}

void shiet_window_win32__get_window_size(void)
{
    return;
}
