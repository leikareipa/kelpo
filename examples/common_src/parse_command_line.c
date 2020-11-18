/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A partial, rudimentary cross-platform getopt() replacement for use by the
 * Kelpo renderer.
 * 
 */

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "parse_command_line.h"

static unsigned CURRENT_IDX = 0;

#define MAX_OPTARG_LENGTH 150
static char LATEST_OPTION_ARGUMENT[MAX_OPTARG_LENGTH + 1]; /* +1 for \0.*/

void kelpo_cliparse_get_params(const int argc,
                               char *const argv[],
                               struct kelpo_cliparse_params_s *const params)
{
    int c = 0;
    while ((c = kelpo_cliparse(argc, argv)) != -1)
    {
        switch (c)
        {
            case 'r':
            {
                const char *const rendererName = kelpo_cliparse_optarg();
                char *const paramRendererName = malloc((strlen(rendererName) + 1));
                strcpy(paramRendererName, rendererName);
                params->rendererName = paramRendererName;
                break;
            }
            case 'v':
            {
                params->vsyncEnabled = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                break;
            }
            case 'w':
            {
                params->windowWidth = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                assert((params->windowWidth != 0u) && "Invalid render width.");
                break;
            }
            case 'h':
            {
                params->windowHeight = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                assert((params->windowHeight != 0u) && "Invalid render height.");
                break;
            }
            case 'b':
            {
                params->windowBPP = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                assert((params->windowBPP != 0u) && "Invalid render bit depth.");
                break;
            }
            case 'd':
            {
                /* The device index is expected to be 1-indexed (device #1 is 1).*/
                params->renderDeviceIdx = strtoul(kelpo_cliparse_optarg(), NULL, 10);
                assert((params->renderDeviceIdx != 0u) && "Invalid render device index.");
                params->renderDeviceIdx--;
                break;
            }
            default: break;
        }
    }

    return;
}

int kelpo_cliparse(const int argc, char *const argv[])
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

const char* kelpo_cliparse_optarg(void)
{
    return LATEST_OPTION_ARGUMENT;
}
