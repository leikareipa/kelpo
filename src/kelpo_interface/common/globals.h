#ifndef KELPO_INTERFACE_COMMON_GLOBALS_H
#define KELPO_INTERFACE_COMMON_GLOBALS_H

#define NUM_ARRAY_ELEMENTS(array) ((size_t)(sizeof(array) / sizeof((array)[0])))

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define KELPO_DEG_TO_RAD(x) (((x) * (M_PI)) / 180.0)

#define KELPO_LERP(x, y, step) ((x) + ((step) * ((y) - (x))))

enum
{
    KELPO_COLOR_FMT_UNKNOWN = 0,
    KELPO_COLOR_FMT_RGBA_8888,
    KELPO_COLOR_FMT_RGB_565,
    KELPO_COLOR_FMT_RGB_555
};

#endif
