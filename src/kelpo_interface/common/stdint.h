/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A rudimentary C89 implementation of stdint.h for the Kelpo renderer.
 * 
 */

#ifndef KELPO_INTERFACE_COMMON_STDINT_H
#define KELPO_INTERFACE_COMMON_STDINT_H

#if defined (_WIN32)
    #if defined (__MINGW32__) || (__WATCOMC__)
        typedef int              int32_t;
        typedef short            int16_t;
        typedef signed char      int8_t;
        typedef unsigned int     uint32_t;
        typedef unsigned short   uint16_t;
        typedef unsigned char    uint8_t;
    #elif defined (__DMC__)
        typedef long             int32_t;
        typedef short            int16_t;
        typedef signed char      int8_t;
        typedef unsigned long    uint32_t;
        typedef unsigned short   uint16_t;
        typedef unsigned char    uint8_t;
    #elif defined (_MSC_VER)
        typedef __int32          int32_t;
        typedef __int16          int16_t;
        typedef __int8           int8_t;
        typedef unsigned __int32 uint32_t;
        typedef unsigned __int16 uint16_t;
        typedef unsigned __int8  uint8_t;
    #else
        #warning "Unknown compiler; blindly defining standard int types."
        typedef int              int32_t;
        typedef short            int16_t;
        typedef signed char      int8_t;
        typedef unsigned int     uint32_t;
        typedef unsigned short   uint16_t;
        typedef unsigned char    uint8_t;
    #endif
#else
    #warning "Unknown platform; blindly defining standard int types."
    typedef int             int32_t;
    typedef short           int16_t;
    typedef signed char     int8_t;
    typedef unsigned int    uint32_t;
    typedef unsigned short  uint16_t;
    typedef unsigned char   uint8_t;
#endif

#endif
