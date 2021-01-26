#ifndef KELPO_INTERFACE_INTERFACE_H
#define KELPO_INTERFACE_INTERFACE_H

#include <string.h>
#include <kelpo_interface/stdint.h>
#include <windef.h>

#define KELPO_INTERFACE_VERSION_MAJOR 0 /* Starting from version 1, bumped when introducing breaking interface changes.*/
#define KELPO_INTERFACE_VERSION_MINOR 8 /* Bumped (or not) when such new functionality is added that doesn't break compatibility with existing implementations of current major version.*/
#define KELPO_INTERFACE_VERSION_PATCH 0 /* Bumped (or not) on minor bug fixes etc.*/

/* Utility function for renderers. Copies the renderer name (src) into an
 * interface object's 'metadata.rendererName' string buffer (dst). Expects that
 * the dst buffer is an array on the stack.*/
#define KELPO_COPY_RENDERER_NAME(dst, src) {\
    const unsigned dstLen = (sizeof(dst) / sizeof(char));\
    const unsigned srcLen = (strlen(src) + 1);\
    strncpy(dst, src, ((srcLen > dstLen)? dstLen : srcLen));\
    dst[dstLen - 1] = '\0';\
}

/* A user-provided function that will receive the renderer window's messages.*/
typedef LRESULT kelpo_custom_window_message_handler_t(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

struct kelpo_polygon_triangle_s;
struct kelpo_polygon_texture_s;

struct kelpo_interface_s
{
    /* A handle to the DLL file from which this interface was loaded, as
       returned by LoadLibraryA().*/
    HMODULE dllHandle;

    struct kelpo_interface_window_s
    {
        uint32_t (*get_handle)(void);

        int (*open)(const unsigned deviceId,
                    const unsigned screenWidth,
                    const unsigned screenHeight,
                    const unsigned screenBPP);

        int (*release)(void);

        int (*set_message_handler)(kelpo_custom_window_message_handler_t *const customHandlerFn);

        int (*process_messages)(void);

        int (*flip_surface)(void);

        int (*is_open)(void);

        /*size*/
    } window;

    struct kelpo_interface_rasterizer_s
    {
        int (*clear_frame)(void);

        int (*upload_texture)(struct kelpo_polygon_texture_s *const texture);

        int (*update_texture)(struct kelpo_polygon_texture_s *const texture);
        
        int (*unload_textures)(void);

        int (*draw_triangles)(struct kelpo_polygon_triangle_s *const triangles,
                              const unsigned numTriangles);

        /*release*/

        /*framebuffer*/
    } rasterizer;

    struct kelpo_interface_metadata_s
    {
        char rendererName[64];
        unsigned rendererVersionMajor;
        unsigned rendererVersionMinor;
        unsigned rendererVersionPatch;
    } metadata;
};

int kelpo_create_interface(const struct kelpo_interface_s **dst,
                           const char *const rendererName);

int kelpo_release_interface(const struct kelpo_interface_s *const kelpoInterface);

const char* kelpo_active_renderer_name(void);

#endif
