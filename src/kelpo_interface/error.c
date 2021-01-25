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
static uint32_t KELPO_ERROR_CODE = KELPOERR_NO_ERROR;

/* The most recent error string, as reported to kelpo_error(). Will be set
 * to NULL if no errors have been reported.*/
static const char *KELPO_ERROR_STRING = NULL;

void kelpo_error_reset(void)
{
    KELPO_ERROR_CODE = KELPOERR_NO_ERROR;
    KELPO_ERROR_STRING = NULL;

    return;
}

uint32_t kelpo_error_code(void)
{
    return KELPO_ERROR_CODE;
}

const char* kelpo_error_string(void)
{
    return KELPO_ERROR_STRING;
}

void kelpo_error(const uint32_t errorCode,
                 const char *const errorString)
{
    KELPO_ERROR_CODE = errorCode;
    KELPO_ERROR_STRING = errorString;

    if (KELPO_ERROR_VERBOSE)
    {
        fprintf(stderr, "KELPO ERROR (%s): %s\n",
                        kelpo_active_renderer_name(),
                        errorString);
    }

    return;
}
