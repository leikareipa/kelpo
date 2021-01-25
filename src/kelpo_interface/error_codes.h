/*
 * 2021 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#ifndef KELPO_INTERFACE_ERROR_CODES_H
#define KELPO_INTERFACE_ERROR_CODES_H

/* An error code of 0 indicates that no error is present.*/
#define KELPOERR_NO_ERROR 0

/* The high 16 bits of a Kelpo error code; defines which sub-system of Kelpo
 * the error relates to. NOTE: The values must start from 1.*/
enum error_code_subsystem_e
{
    KELPOERR_INTERFACE = 1,
    KELPOERR_RASTERIZER,
    KELPOERR_SURFACE,
    KELPOERR_AUXILIARY,
    KELPOERR_WINDOW,
    KELPOERR_DLL
};

/* The low 16 bits of a Kelpo error code; defines the error's category.
 * NOTE: The values must start from 1.*/
enum error_code_category_e
{
    KELPOERR_FAILURE_TO_INITIALIZE = 1, /* E.g. could not allocate memory for an object or to successfully call its constructor.*/
    KELPOERR_FAILURE_TO_RELEASE /* E.g. could not release an object's memory or to unload a DLL library.*/
};

/* Generates an error code that can be fed into kelpo_error().*/
#define KELPOERR(subsystem, category) (uint32_t)(((uint16_t)(subsystem) << 16) | (uint16_t)(category))

#endif
