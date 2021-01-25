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

/* Returns the most recent error code, or KELPOERR_NO_ERROR if no error has
 * occurred since the error code was last reset (by calling kelpo_error_reset()).*/
enum kelpo_error_code_e kelpo_error_code(void);

/* Reset the error code. Immediately after calling this function and until an
 * error occurs, kelpo_error_code() will return KELPOERR_NO_ERROR.*/
void kelpo_error_reset(void);

/* Used by Kelpo's internals to report runtime errors.*/
void kelpo_error(const enum kelpo_error_code_e errorCode);

#endif
