/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "parse_command_line.h"

#define MAX_OPTARG_LENGTH 150
static char LATEST_OPTION_ARGUMENT[MAX_OPTARG_LENGTH + 1]; /* +1 for \0.*/

static int cliparse(const int argc, char *const argv[])
{
    static unsigned currentArgIdx = 0;

    while (++currentArgIdx)
    {
        /* If there's at most one more element remaining, we've finished parsing,
         * since the remaining element can't contain a valid option (which requires
         * two elements).*/
        if (currentArgIdx >= (argc - 1))
        {
            currentArgIdx = 0;
            break;
        }

        if ((strlen(argv[currentArgIdx]) == 2) &&
            (argv[currentArgIdx][0] == '-'))
        {
            strncpy(LATEST_OPTION_ARGUMENT, argv[++currentArgIdx], MAX_OPTARG_LENGTH);
            return (int)argv[currentArgIdx-1][1];
        }
    }

    return -1;
}


void cliparse_get_params(const int argc,
                         char *const argv[],
                         struct cliparse_params_s *const params)
{
    int c = 0;
    while ((c = cliparse(argc, argv)) != -1)
    {
        switch (c)
        {
            case 'r':
            {
                const char *const rendererName = LATEST_OPTION_ARGUMENT;
                char *const paramRendererName = malloc((strlen(rendererName) + 1));
                strcpy(paramRendererName, rendererName);
                params->rendererName = paramRendererName;
                break;
            }
            case 'v':
            {
                params->vsyncEnabled = strtoul(LATEST_OPTION_ARGUMENT, NULL, 10);
                break;
            }
            case 'w':
            {
                params->windowWidth = strtoul(LATEST_OPTION_ARGUMENT, NULL, 10);
                assert((params->windowWidth != 0u) && "Invalid render width.");
                break;
            }
            case 'h':
            {
                params->windowHeight = strtoul(LATEST_OPTION_ARGUMENT, NULL, 10);
                assert((params->windowHeight != 0u) && "Invalid render height.");
                break;
            }
            case 'b':
            {
                params->windowBPP = strtoul(LATEST_OPTION_ARGUMENT, NULL, 10);
                assert((params->windowBPP != 0u) && "Invalid render bit depth.");
                break;
            }
            case 'd':
            {
                /* The device index is expected to be 1-indexed (device #1 is 1).*/
                params->renderDeviceIdx = strtoul(LATEST_OPTION_ARGUMENT, NULL, 10);
                assert((params->renderDeviceIdx != 0u) && "Invalid render device index.");
                params->renderDeviceIdx--;
                break;
            }
            default: break;
        }
    }

    return;
}
