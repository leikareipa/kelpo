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

/* A list of the errors reported via kelpo_error() since the last time the list
 * was reset. TODO: Maybe implement a dynamic list instead of a static one.*/
#define MAX_NUM_REPORTED_ERRORS 32
static unsigned NUM_REPORTED_ERRORS = 0;
static enum kelpo_error_code_e REPORTED_ERRORS[MAX_NUM_REPORTED_ERRORS];

void kelpo_error_reset(void)
{
    NUM_REPORTED_ERRORS = 0;
    memset(REPORTED_ERRORS, KELPOERR_NO_ERROR, sizeof(REPORTED_ERRORS));

    return;
}

enum kelpo_error_code_e kelpo_error_peek(void)
{
    return !NUM_REPORTED_ERRORS
           ? KELPOERR_NO_ERROR
           : REPORTED_ERRORS[NUM_REPORTED_ERRORS - 1];
}

enum kelpo_error_code_e kelpo_error_code(void)
{
    return !NUM_REPORTED_ERRORS
           ? KELPOERR_NO_ERROR
           : REPORTED_ERRORS[--NUM_REPORTED_ERRORS];
}

void kelpo_error(enum kelpo_error_code_e errorCode)
{
    if (NUM_REPORTED_ERRORS >= MAX_NUM_REPORTED_ERRORS)
    {
        return;
    }

    if (NUM_REPORTED_ERRORS == (MAX_NUM_REPORTED_ERRORS - 1))
    {
        errorCode = KELPOERR_TOO_MANY_ERRORS;
    }

    REPORTED_ERRORS[NUM_REPORTED_ERRORS++] = errorCode;

    if (KELPO_ERROR_VERBOSE)
    {
        /* TODO: Error string instead of error code.*/
        fprintf(stderr, "KELPO ERROR (%s): %u\n",
                        kelpo_active_renderer_name(),
                        errorCode);
    }

    return;
}
