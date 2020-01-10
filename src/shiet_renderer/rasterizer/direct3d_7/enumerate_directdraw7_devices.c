/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 */

#include <stdio.h>
#include <shiet_renderer/rasterizer/direct3d_7/enumerate_directdraw7_devices.h>
 
/* Called by DirectDrawEnumerateEx() for each DirectDraw 7 device on the system.
 * Appends the device's info to the device list.*/
static BOOL WINAPI device_enum_callback(GUID* deviceGUID,
                                        LPSTR deviceDescription,
                                        LPSTR deviceName,
                                        VOID* deviceList,
                                        HMONITOR)
{
    struct shiet_directdraw7_device_list_s *list = (struct shiet_directdraw7_device_list_s*)deviceList;
    struct shiet_directdraw7_device_list_entry_s *newDevice = (struct shiet_directdraw7_device_list_entry_s*)
                                                              malloc(sizeof(struct shiet_directdraw7_device_list_entry_s));

    *list->end = newDevice;
    list->end = &newDevice->next;

    memset(newDevice, 0, sizeof(newDevice[0]));
    newDevice->guid = deviceGUID;
    strncpy(newDevice->description, deviceDescription, (SHIET_DIRECTDRAW7_DEVICE_LIST_ENTRY_STRING_LENGTH-1));

    list->count++;

    /* Continue enumerating.*/
    return 1;
}

shiet_directdraw7_device_list_s shiet_enumerate_directdraw7_devices(void)
{
    struct shiet_directdraw7_device_list_s deviceList;

    memset(&deviceList, 0, sizeof(struct shiet_directdraw7_device_list_s));
    deviceList.end = &deviceList.root;

    DirectDrawEnumerateEx(device_enum_callback, (void*)&deviceList, 
                          (DDENUM_ATTACHEDSECONDARYDEVICES |
                           DDENUM_DETACHEDSECONDARYDEVICES |
                           DDENUM_NONDISPLAYDEVICES));

    return deviceList;
}

GUID* shiet_guid_of_directdraw7_device(unsigned deviceIdx)
{
    struct shiet_directdraw7_device_list_s deviceList = shiet_enumerate_directdraw7_devices();
    struct shiet_directdraw7_device_list_entry_s *it = deviceList.root;
    GUID *guid = NULL;

    while (it)
    {
        struct shiet_directdraw7_device_list_entry_s *entry = it;

        if (deviceIdx-- == 0)
        {
            guid = entry->guid;
        }

        it = entry->next;
        free(entry);
    }

    return guid;
}
