/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Note: The DirectX 6 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <kelpo_renderer/surface/directdraw_6/enumerate_directdraw_6_devices.h>
 
/* Called by DirectDrawEnumerateEx() for each DirectDraw 5 device on the system.
 * Appends the device's info to the device list.*/
static BOOL WINAPI device_enum_callback(GUID *deviceGUID,
                                        LPSTR deviceDescription,
                                        LPSTR deviceName,
                                        VOID *deviceList)
{
    struct kelpo_directdraw6_device_list_s *list = (struct kelpo_directdraw6_device_list_s*)deviceList;
    struct kelpo_directdraw6_device_list_entry_s *newDevice = (struct kelpo_directdraw6_device_list_entry_s*)
                                                              malloc(sizeof(struct kelpo_directdraw6_device_list_entry_s));

    *list->end = newDevice;
    list->end = &newDevice->next;

    memset(newDevice, 0, sizeof(newDevice[0]));
    strncpy(newDevice->description, deviceDescription, KELPO_DIRECTDRAW6_DEVICE_LIST_ENTRY_STRING_LENGTH);
    if (deviceGUID)
    {
        newDevice->guid = *deviceGUID;
    }

    list->count++;

    /* Continue enumerating.*/
    return 1;
}

struct kelpo_directdraw6_device_list_s kelpo_enumerate_directdraw6_devices(void)
{
    struct kelpo_directdraw6_device_list_s deviceList;

    memset(&deviceList, 0, sizeof(struct kelpo_directdraw6_device_list_s));
    deviceList.end = &deviceList.root;

    DirectDrawEnumerate(device_enum_callback, (void*)&deviceList);

    return deviceList;
}

GUID kelpo_directdraw6_device_guid(unsigned deviceIdx)
{
    struct kelpo_directdraw6_device_list_s deviceList = kelpo_enumerate_directdraw6_devices();
    struct kelpo_directdraw6_device_list_entry_s *it = deviceList.root;
    GUID guid;
    int guidFound = 0;

    memset(&guid, 0, sizeof(guid));

    while (it)
    {
        struct kelpo_directdraw6_device_list_entry_s *entry = it;

        if (deviceIdx-- == 0)
        {
            guid = entry->guid;
            guidFound = 1;
        }

        it = entry->next;
        free(entry);
    }

    assert(guidFound && "DirectDraw 5: Unknown device.");

    return guid;
}
