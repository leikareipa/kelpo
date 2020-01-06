/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A partial, rudimentary cross-platform getopt() replacement for use by the shiet
 * renderer.
 * 
 */

#include <string.h>
#include "parse_command_line.h"

static unsigned CURRENT_IDX = 0;

#define MAX_OPTARG_LENGTH 150
static char LATEST_OPTION_ARGUMENT[MAX_OPTARG_LENGTH + 1]; /* +1 for \0.*/

int shiet_cliparse(const int argc, char *const argv[])
{
    while (++CURRENT_IDX)
    {
        /* If there's at most one more element remaining, we've finished parsing,
         * since the remaining element can't contain a valid option (which requires
         * two elements).*/
        if (CURRENT_IDX >= (argc - 1))
        {
            CURRENT_IDX = 0;
            break;
        }

        if ((strlen(argv[CURRENT_IDX]) == 2) &&
            (argv[CURRENT_IDX][0] == '-'))
        {
            strncpy(LATEST_OPTION_ARGUMENT, argv[++CURRENT_IDX], MAX_OPTARG_LENGTH);
            return (int)argv[CURRENT_IDX-1][1];
        }
    }

    return -1;
}

const char* shiet_cliparse_optarg(void)
{
    return LATEST_OPTION_ARGUMENT;
}
