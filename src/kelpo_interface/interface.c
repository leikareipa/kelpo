#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "kelpo_interface/interface.h"

#include <windows.h>

typedef void *(*dll_import_fn_t)(struct kelpo_interface_s *const);

struct kelpo_interface_s kelpo_create_interface(const char *const rasterizerName)
{
    const char *dllFilename = NULL;
    dll_import_fn_t get_kelpo_interface = NULL;
    struct kelpo_interface_s kelpoInterface = {0};

         if (strcmp(rasterizerName, "opengl_1_2") == 0) dllFilename = "kelpo_renderer_opengl_1_2.dll";
    else if (strcmp(rasterizerName, "opengl_3_0") == 0) dllFilename = "kelpo_renderer_opengl_3_0.dll";
    else if (strcmp(rasterizerName, "glide_3")    == 0) dllFilename = "kelpo_renderer_glide_3.dll";
    else if (strcmp(rasterizerName, "direct3d_5") == 0) dllFilename = "kelpo_renderer_direct3d_5.dll";
    else if (strcmp(rasterizerName, "direct3d_6") == 0) dllFilename = "kelpo_renderer_direct3d_6.dll";
    else if (strcmp(rasterizerName, "direct3d_7") == 0) dllFilename = "kelpo_renderer_direct3d_7.dll";

    assert(dllFilename && "Unknown renderer name.");

    kelpoInterface.dllHandle = LoadLibraryA(dllFilename);
    assert(kelpoInterface.dllHandle && "Could not load the renderer's DLL.");

    get_kelpo_interface = (dll_import_fn_t)GetProcAddress(kelpoInterface.dllHandle, "export_interface");
    assert(get_kelpo_interface && "Could not import the renderer from the DLL.");

    get_kelpo_interface(&kelpoInterface);
    assert((kelpoInterface.metadata.rendererVersionMajor == KELPO_INTERFACE_VERSION_MAJOR) &&
           "The renderer's version is not compatible with this version of Kelpo.");

    return kelpoInterface;
}

/* Returns 1 on success; 0 otherwise.*/
int kelpo_release_interface(struct kelpo_interface_s *const kelpoInterface)
{
    assert(kelpoInterface && "Received a NULL Kelpo interface.");

    if (kelpoInterface->window.is_open())
    {
        kelpoInterface->window.destroy();
    }

    if (kelpoInterface->dllHandle &&
        !FreeLibrary(kelpoInterface->dllHandle))
    {
        fprintf(stderr, "ERROR: Could not release the Kelpo renderer library.\n");

        return 0;
    }

    memset(kelpoInterface, 0, sizeof(*kelpoInterface));
    
    return 1;
}

#undef DLL_FUNC_ADDRESS
