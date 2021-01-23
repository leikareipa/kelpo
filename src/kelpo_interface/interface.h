#ifndef KELPO_INTERFACE_INTERFACE_H
#define KELPO_INTERFACE_INTERFACE_H

#include <kelpo_interface/stdint.h>
#include <windef.h>

#define KELPO_INTERFACE_VERSION_MAJOR 0 /* Starting from version 1, bumped when introducing breaking interface changes.*/
#define KELPO_INTERFACE_VERSION_MINOR 6 /* Bumped (or not) when such new functionality is added that doesn't break compatibility with existing implementations of current major version.*/
#define KELPO_INTERFACE_VERSION_PATCH 0 /* Bumped (or not) on minor bug fixes etc.*/

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

        int (*destroy)(void);

        void (*set_message_handler)(kelpo_custom_window_message_handler_t *const customHandlerFn);

        void (*process_messages)(void);

        void (*flip_surface)(void);

        int (*is_open)(void);

        /*size*/
    } window;

    struct kelpo_interface_rasterizer_s
    {
        void (*clear_frame)(void);

        void (*upload_texture)(struct kelpo_polygon_texture_s *const texture);

        void (*update_texture)(struct kelpo_polygon_texture_s *const texture);
        
        void (*unload_textures)(void);

        void (*draw_triangles)(struct kelpo_polygon_triangle_s *const triangles,
                               const unsigned numTriangles);

        /*release*/

        /*framebuffer*/
    } rasterizer;

    struct kelpo_interface_metadata_s
    {
        const char *rendererName;
        unsigned rendererVersionMajor;
        unsigned rendererVersionMinor;
        unsigned rendererVersionPatch;
    } metadata;
};

const struct kelpo_interface_s* kelpo_create_interface(const char *const rendererName);

int kelpo_release_interface(const struct kelpo_interface_s *const kelpoInterface);

#endif
