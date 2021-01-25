/*
 * 2021 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#include <stdio.h>
#include <stddef.h>
#include <kelpo_interface/error.h>
#include <kelpo_interface/interface.h>

/* If true, error messages will be printed into the console also. Otherwise,
 * errors will only populate the internal error buffers.*/
int KELPO_ERROR_VERBOSE = 1;

/* The most recent error code, as reported to kelpo_error(). Will evaluate
 * to true if one or more errors have been reported since its value was set
 * to KELPOERR_NO_ERROR, and false if not.*/
static enum kelpo_error_code_e KELPO_ERROR_CODE = KELPOERR_NO_ERROR;

void kelpo_error_reset(void)
{
    KELPO_ERROR_CODE = KELPOERR_NO_ERROR;

    return;
}

enum kelpo_error_code_e kelpo_error_code(void)
{
    return KELPO_ERROR_CODE;
}

void kelpo_error(const enum kelpo_error_code_e errorCode)
{
    KELPO_ERROR_CODE = errorCode;

    if (KELPO_ERROR_VERBOSE)
    {
        /* TODO: Error string instead of error code.*/
        fprintf(stderr, "KELPO ERROR (%s): %u\n",
                        kelpo_active_renderer_name(),
                        errorCode);
    }

    return;
}
