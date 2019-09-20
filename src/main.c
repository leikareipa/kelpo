#include <stdio.h>
#include "shiet/common/globals.h"
#include "shiet/interface.h"

int main(void)
{
    char windowTitle[64];

    struct shiet_interface_s shiet = shiet_create_interface("opengl");

    snprintf(windowTitle, NUM_ARRAY_ELEMENTS(windowTitle),
             "shiet %d.%d.%d", shiet.metadata.shietMajorVersion,
                               shiet.metadata.shietMinorVersion,
                               shiet.metadata.shietPatchVersion);

    shiet.initialize(640, 480, 16, windowTitle);

    while (shiet.window.is_window_open())
    {
        shiet.rasterize.clear_frame();
        shiet.window.update_window();
    }

    return 0;
}
