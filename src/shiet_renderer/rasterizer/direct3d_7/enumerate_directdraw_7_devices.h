/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Note: The Direct3D 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of shiet.
 * 
 */

#ifndef SHIET_RENDERER_RASTERIZER_DIRECT3D_7_ENUMERATE_DIRECTDRAW_7_DEVICES_H
#define SHIET_RENDERER_RASTERIZER_DIRECT3D_7_ENUMERATE_DIRECTDRAW_7_DEVICES_H

#include <d3d.h>

#define SHIET_DIRECTDRAW7_DEVICE_LIST_ENTRY_STRING_LENGTH 60

struct shiet_directdraw7_device_list_entry_s
{
    GUID guid;
    char description[SHIET_DIRECTDRAW7_DEVICE_LIST_ENTRY_STRING_LENGTH + 1]; /* +1 for \0.*/

    struct shiet_directdraw7_device_list_entry_s *next;
};

/* A linked list of DirectDraw 7 devices.*/
struct shiet_directdraw7_device_list_s
{
    /* First device in the list.*/
    struct shiet_directdraw7_device_list_entry_s *root;

    /* Points to a pointer to which the next new device entry should be allocated
     * (the one-past-last entry).*/
    struct shiet_directdraw7_device_list_entry_s **end;

    /* Total number of devices in this list.*/
    unsigned count;
};

/* Returns a list of all DirectDraw 7 devices on this PC.*/
shiet_directdraw7_device_list_s shiet_enumerate_directdraw7_devices(void);

/* Returns the GUID of the idx'th DirectDraw 7 device on this PC.*/
GUID shiet_directdraw7_device_guid(unsigned deviceIdx);

#endif
