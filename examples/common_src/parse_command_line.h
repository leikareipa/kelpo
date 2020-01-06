/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A partial, rudimentary cross-platform getopt() replacement for use by the shiet
 * renderer.
 * 
 * Usage:
 * 
 *   - Call shiet_cliparse() in a loop until it returns -1 to signal that there
 *     are no more options on the command-line. The function returns each option
 *     in turn - e.g. the command-line string
 * 
 *         "-r something -w another"
 * 
 *     returns 'r' on the first call (shiet_cliparse_optarg() returns the "something"
 *     part), 'w' on the second call (shiet_cliparse_optarg() now returns "another"),
 *     and -1 on the third call.
 * 
 *     Note that only options of the form "-x xxxx" are recognized.
 *     Note also this this is a quick implementation that likely has bugs etc.
 * 
 */

#ifndef SHIET_EXAMPLES_COMMON_PARSE_COMMAND_LINE_H
#define SHIET_EXAMPLES_COMMON_PARSE_COMMAND_LINE_H

int shiet_cliparse(const int argc, char *const argv[]);
const char* shiet_cliparse_optarg(void);

#endif
