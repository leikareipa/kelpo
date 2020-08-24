#ifndef KELPO_AUXILIARY_MISC_H
#define KELPO_AUXILIARY_MISC_H

#define KELPOA_NUM_ARRAY_ELEMENTS(array) ((size_t)(sizeof(array) / sizeof((array)[0])))

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define KELPOA_DEG_TO_RAD(x) (((x) * (M_PI)) / 180.0)

#define KELPOA_LERP(x, y, step) ((x) + ((step) * ((y) - (x))))

#endif
