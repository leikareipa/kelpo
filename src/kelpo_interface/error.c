/*
 * 2021 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#include <stdio.h>
#include <assert.h>
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

unsigned kelpo_error_reset(void)
{
    const unsigned numErrors = NUM_REPORTED_ERRORS;

    NUM_REPORTED_ERRORS = 0;
    memset(REPORTED_ERRORS, KELPOERR_ALL_GOOD, sizeof(REPORTED_ERRORS));

    return numErrors;
}

enum kelpo_error_code_e kelpo_error_peek(void)
{
    assert((NUM_REPORTED_ERRORS <= MAX_NUM_REPORTED_ERRORS) &&
           "Malformed error queue.");

    return !NUM_REPORTED_ERRORS
           ? KELPOERR_ALL_GOOD
           : REPORTED_ERRORS[NUM_REPORTED_ERRORS - 1];
}

enum kelpo_error_code_e kelpo_error_code(void)
{
    assert((NUM_REPORTED_ERRORS <= MAX_NUM_REPORTED_ERRORS) &&
           "Malformed error queue.");

    return !NUM_REPORTED_ERRORS
           ? KELPOERR_ALL_GOOD
           : REPORTED_ERRORS[--NUM_REPORTED_ERRORS];
}

void kelpo_report_error(enum kelpo_error_code_e errorCode,
                        const char *const sourceFile,
                        const int lineNumber)
{
    assert((errorCode != KELPOERR_ALL_GOOD) &&
           "This is a reserved error meant for internal use only. It can't be reported.");

    assert((NUM_REPORTED_ERRORS <= MAX_NUM_REPORTED_ERRORS) &&
           "Malformed error queue.");

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
        fprintf(stderr, "Kelpo v%u.%u.%u error in %s:%d: %d / %s\n",
                        KELPO_INTERFACE_VERSION_MAJOR,
                        KELPO_INTERFACE_VERSION_MINOR,
                        KELPO_INTERFACE_VERSION_PATCH,
                        sourceFile,
                        lineNumber,
                        errorCode,
                        kelpo_error_string(errorCode));
    }

    return;
}

const char* kelpo_error_string(const enum kelpo_error_code_e errorCode)
{
    switch (errorCode)
    {
        case KELPOERR_ALL_GOOD:
            return "No errors";

        case KELPOERR_TOO_MANY_ERRORS:
            return "Too many errors";

        case KELPOERR_Z_BUFFERING_NOT_SUPPORTED:
            return "Z-buffering not supported";

        case KELPOERR_VSYNC_CONTROL_NOT_SUPPORTED:
            return "Vsync not supported";

        case KELPOERR_DISPLAY_MODE_NOT_SUPPORTED:
            return "Display mode not supported";

        case KELPOERR_OUT_OF_VIDEO_MEMORY:
            return "Out of video memory";

        case KELPOERR_API_CALL_FAILED:
            return "API call failed";

        case KELPOERR_RENDERER_NOT_AVAILABLE:
            return "Renderer not available";

        case KELPOERR_RENDERER_NOT_COMPATIBLE_WITH_INTERFACE:
            return "Renderer not compatible with interface";

        default:
            return "Unnamed error";
    }
}
