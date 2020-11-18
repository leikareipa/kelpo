/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 * Parses command-line parameters for Kelpo render examples.
 * 
 */

#ifndef KELPO_EXAMPLES_COMMON_SRC_PARSE_COMMAND_LINE_H
#define KELPO_EXAMPLES_COMMON_SRC_PARSE_COMMAND_LINE_H

struct cliparse_params_s
{
    const char *rendererName;

    /* An index in an enumeration of API-compatible devices on the system,
     * identifying the devide to be used in rendering.*/
    unsigned renderDeviceIdx;

    /* If set to 1, we'll request the renderer to use vsync. Otherwise, we'll
     * ask for vsync to be off. On some hardware, this option will have no
     * effect, however.*/
    unsigned vsyncEnabled;

    /* The resolution and color depth of the render window.*/
    unsigned windowWidth;
    unsigned windowHeight;
    unsigned windowBPP;
};

void cliparse_get_params(const int argc, char *const argv[], struct cliparse_params_s *const params);

#endif
