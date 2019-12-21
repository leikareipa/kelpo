#ifndef SHIET_GLOBALS_H_
#define SHIET_GLOBALS_H_

#define NUM_ARRAY_ELEMENTS(array) ((size_t)(sizeof(array) / sizeof((array)[0])))

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define DEG_TO_RAD(x) (((x) * (M_PI)) / 180.0)

#endif
