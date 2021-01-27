/*
 * 2021 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#ifndef KELPO_INTERFACE_ERROR_H
#define KELPO_INTERFACE_ERROR_H

#include <kelpo_interface/error_codes.h>
#include <kelpo_interface/stdint.h>

/* If set to 1, errors submitted to kelpo_error() will also be printed into
 * the console (e.g. stderr). Otherwise, no console output will be generated.*/
extern int KELPO_ERROR_VERBOSE;

/* Returns and removes the most recent error code. To retrieve all errors codes,
 * call this function in a loop until KELPOERR_ALL_GOOD is returned. Error codes
 * are accumulated starting from either the latest call to kelpo_error_reset()
 * or from program startup if no such call has been made.*/
enum kelpo_error_code_e kelpo_error_code(void);

/* Returns a human-readable string corresponding to the given error code.*/
const char* kelpo_error_string(const enum kelpo_error_code_e errorCode);

/* Returns the most recent error code, or KELPOERR_ALL_GOOD if there are no
 * errors in the error queue (e.g. due to no errors having been reported, or
 * all reported errors having been removed via kelpo_error_code() or
 * kelpo_error_reset()).
 * 
 * Unlike kelpo_error_code(), this function doesn't remove the returned error
 * code from the queue, and can only reveal the most recent error code.*/
enum kelpo_error_code_e kelpo_error_peek(void);

/* Removes all accumulated error codes. Returns the number of errors removed.
 * Immediately after calling this function and until one or more errors occur,
 * kelpo_error_code() will return KELPOERR_ALL_GOOD.*/
unsigned kelpo_error_reset(void);

/* Convenience shortcut for kelpo_report_error().*/
#define kelpo_error(/*enum kelpo_error_code_e*/ errorCode) kelpo_report_error((errorCode), __FILE__, __LINE__);

/* Used by Kelpo's internals to report errors. Use kelpo_error_code() and/or
 * kelpo_error_peek() to query them.*/
void kelpo_report_error(enum kelpo_error_code_e errorCode,
                        const char *const sourceFile, /* E.g. __FILE__.*/
                        const int lineNumber); /* E.g. __LINE__.*/

#endif
