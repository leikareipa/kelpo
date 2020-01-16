/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A partial, rudimentary cross-platform getopt() replacement for use by the Kelpo
 * renderer.
 * 
 * Usage:
 * 
 *   - Call kelpo_cliparse() in a loop until it returns -1 to signal that there
 *     are no more options on the command-line. The function returns each option
 *     in turn - e.g. the command-line string
 * 
 *         "-r something -w another"
 * 
 *     returns 'r' on the first call (kelpo_cliparse_optarg() returns the "something"
 *     part), 'w' on the second call (kelpo_cliparse_optarg() now returns "another"),
 *     and -1 on the third call.
 * 
 *     Note that only options of the form "-x xxxx" are recognized.
 *     Note also this this is a quick implementation that likely has bugs etc.
 * 
 */

#ifndef KELPO_EXAMPLES_COMMON_SRC_PARSE_COMMAND_LINE_H
#define KELPO_EXAMPLES_COMMON_SRC_PARSE_COMMAND_LINE_H

int kelpo_cliparse(const int argc, char *const argv[]);
const char* kelpo_cliparse_optarg(void);

#endif
