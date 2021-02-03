/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#include <stddef.h>

unsigned framerate_estimate(void)
{
    static unsigned numFramesCounted = 0;
    static unsigned framesPerSecond = 0;
    static unsigned frameRateTimer = 0;

    numFramesCounted++;

    if (!frameRateTimer ||
        ((time(NULL) - frameRateTimer) >= 2))
    {
        framesPerSecond = (numFramesCounted / (time(NULL) - frameRateTimer));
        frameRateTimer = time(NULL);
        numFramesCounted = 0;
    }

    return framesPerSecond;
}
