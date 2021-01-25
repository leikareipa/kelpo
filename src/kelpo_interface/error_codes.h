/*
 * 2021 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#ifndef KELPO_INTERFACE_ERROR_CODES_H
#define KELPO_INTERFACE_ERROR_CODES_H

enum kelpo_error_code_e
{
    KELPOERR_NO_ERROR = 0,

    KELPOERR_Z_BUFFERING_NOT_SUPPORTED,
    KELPOERR_VSYNC_CONTROL_NOT_SUPPORTED,

    KELPOERR_DDRAW_COULDNT_SET_DISPLAY_MODE,
    KELPOERR_DDRAW_COULDNT_LOCK_SURFACE,
    KELPOERR_DDRAW_COULDNT_UNLOCK_SURFACE,
    KELPOERR_DDRAW_COULDNT_CREATE_SURFACE,
    KELPOERR_DDRAW_COULDNT_CREATE_DDRAW,
    KELPOERR_DDRAW_COULDNT_ATTACH_SURFACE,
    KELPOERR_DDRAW_COULDNT_FLIP_SURFACE,
    KELPOERR_DDRAW_COULDNT_CREATE_INTERFACE,
    KELPOERR_DDRAW_COULDNT_CREATE_D3D_DEVICE,
    KELPOERR_DDRAW_COULDNT_SET_D3D_RENDER_TARGET,
    KELPOERR_DDRAW_COULDNT_SET_COOPERATIVE_LEVEL,
    KELPOERR_DDRAW_SURFACE_NOT_AVAILABLE,
    KELPOERR_DDRAW_INTERFACE_NOT_AVAILABLE,

    KELPOERR_D3D_COULDNT_BEGIN_SCENE,
    KELPOERR_D3D_COULDNT_QUERY_DEVICE_CAPS,
    KELPOERR_D3D_COULDNT_CREATE_VIEWPORT,

    KELPOERR_GLIDE_OUT_OF_VRAM,
    KELPOERR_GLIDE_COULDNT_INITIALIZE_RENDER_CONTEXT,

    KELPOERR_OGL_COULDNT_SET_DISPLAY_MODE,
    KELPOERR_OGL_COULDNT_INITIALIZE_RENDER_CONTEXT,
    KELPOERR_OGL_COULDNT_RELEASE_RENDER_CONTEXT,
    KELPOERR_OGL_REQUIRED_EXTENSION_NOT_SUPPORTED,

    KELPOERR_WIN32_COULDNT_CREATE_WINDOW,
    KELPOERR_WIN32_COULDNT_RELEASE_WINDOW,
    KELPOERR_WIN32_COULDNT_UNREGISTER_WINDOW_CLASS,

    KELPOERR_INTERFACE_COULDNT_RELEASE_RENDERER_DLL
};

#endif
