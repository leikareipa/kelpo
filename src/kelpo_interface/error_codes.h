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
    KELPOERR_ALL_GOOD = 0, /* This error code must always remain 0 and is reserved for internal use. It indicates that there are no errors in the error queue.*/
    KELPOERR_TOO_MANY_ERRORS,
    KELPOERR_Z_BUFFERING_NOT_SUPPORTED,
    KELPOERR_VSYNC_CONTROL_NOT_SUPPORTED,
    KELPOERR_DISPLAY_MODE_NOT_SUPPORTED,
    KELPOERR_OUT_OF_VIDEO_MEMORY,
    KELPOERR_API_CALL_FAILED, /* A call to an API function, e.g. glVertex3f() (OpenGL), failed. In some cases, the relevant API error code will have been printed into stderr.*/
    KELPOERR_RENDERER_NOT_AVAILABLE, /* The given Kelpo renderer, e.g. Glide, isn't available/supported on the system.*/
    KELPOERR_RENDERER_NOT_COMPATIBLE_WITH_INTERFACE /* The renderer was compiled for a different version of the interface.*/
};

#endif
