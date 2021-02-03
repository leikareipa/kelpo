/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: Kelpo
 * 
 */

#ifndef KELPO_EXAMPLES_COMMON_SRC_FRAMERATE_ESTIMATE_H
#define KELPO_EXAMPLES_COMMON_SRC_FRAMERATE_ESTIMATE_H

/* Call this function once per frame and it'll return an estimate of the
 * current frame rate (number of frames per second).*/
unsigned framerate_estimate(void);

#endif
