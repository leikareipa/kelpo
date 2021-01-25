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
 * call this function in a loop until KELPOERR_NO_ERROR is returned. Error codes
 * are accumulated starting from either the latest call to kelpo_error_reset()
 * or from program startup if no such call has been made.*/
enum kelpo_error_code_e kelpo_error_code(void);

/* Returns the most recent error code, or KELPOERR_NO_ERROR if there are no
 * errors in the error queue (e.g. due to no errors having been reported, or
 * all reported errors having been removed via kelpo_error_code() or
 * kelpo_error_reset()).
 * 
 * Unlike kelpo_error_code(), this function doesn't remove the returned error
 * code from the queue, and can only reveal the most recent error code.*/
enum kelpo_error_code_e kelpo_error_peek(void);

/* Removes all accumulated error codes. Immediately after calling this function
 * and until one or more errors occur, kelpo_error_code() will return
 * KELPOERR_NO_ERROR.*/
void kelpo_error_reset(void);

/* Used by Kelpo's internals to report errors. Use kelpo_error_code() to query
 * them.*/
void kelpo_error(enum kelpo_error_code_e errorCode);

#endif
