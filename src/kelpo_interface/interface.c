#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <kelpo_interface/interface.h>
#include <kelpo_interface/error.h>

#include <windows.h>

typedef void *(*dll_import_fn_t)(struct kelpo_interface_s *const);

/* The current render interface. Created/initialized by the most recent call to
 * kelpo_create_interface(). Only one interface can be active at a time.*/
static struct kelpo_interface_s ACTIVE_INTERFACE = {0};

/* Initializes a Kelpo interface for the given renderer, and sets 'dst' to point
 * to that interface. Returns 1 on success; 0 otherwise.*/
int kelpo_create_interface(const struct kelpo_interface_s **dst,
                           const char *const rendererName)
{
    const char *dllFilename = NULL;
    dll_import_fn_t get_kelpo_interface = NULL;

    /* If we currently have an active interface that hasn't yet been released.*/
    if (ACTIVE_INTERFACE.dllHandle &&
        !kelpo_release_interface(&ACTIVE_INTERFACE))
    {
        return 0;
    }

    if      (strcmp(rendererName, "opengl_1_2") == 0) dllFilename = "kelpo_renderer_opengl_1_2.dll";
    else if (strcmp(rendererName, "opengl_3_0") == 0) dllFilename = "kelpo_renderer_opengl_3_0.dll";
    else if (strcmp(rendererName, "glide_3")    == 0) dllFilename = "kelpo_renderer_glide_3.dll";
    else if (strcmp(rendererName, "direct3d_5") == 0) dllFilename = "kelpo_renderer_direct3d_5.dll";
    else if (strcmp(rendererName, "direct3d_6") == 0) dllFilename = "kelpo_renderer_direct3d_6.dll";
    else if (strcmp(rendererName, "direct3d_7") == 0) dllFilename = "kelpo_renderer_direct3d_7.dll";

    assert(dllFilename && "Unknown renderer name.");

    ACTIVE_INTERFACE.dllHandle = LoadLibraryA(dllFilename);
    assert(ACTIVE_INTERFACE.dllHandle && "Failed to load the renderer's DLL.");

    get_kelpo_interface = (dll_import_fn_t)GetProcAddress(ACTIVE_INTERFACE.dllHandle, "export_interface");
    assert(get_kelpo_interface && "Failed to import the renderer from the DLL.");

    get_kelpo_interface(&ACTIVE_INTERFACE);
    assert((ACTIVE_INTERFACE.metadata.rendererVersionMajor == KELPO_INTERFACE_VERSION_MAJOR) &&
           "The renderer's version is not compatible with this version of Kelpo.");

    *dst = &ACTIVE_INTERFACE;
    return 1;
}

/* Returns 1 on success; 0 otherwise.*/
int kelpo_release_interface(const struct kelpo_interface_s *const kelpoInterface)
{
    assert((kelpoInterface == &ACTIVE_INTERFACE) && 
           "Can't release an interface that isn't active.");
    
    if (!ACTIVE_INTERFACE.rasterizer.unload_textures() ||
        !ACTIVE_INTERFACE.window.release())
    {
        return 0;
    }

    if (kelpoInterface->dllHandle &&
        !FreeLibrary(ACTIVE_INTERFACE.dllHandle))
    {
        kelpo_error(KELPOERR_API_CALL_FAILED);
        return 0;
    }

    memset(&ACTIVE_INTERFACE, 0, sizeof(ACTIVE_INTERFACE));

    return 1;
}

const char* kelpo_active_renderer_name(void)
{
    return strlen(ACTIVE_INTERFACE.metadata.rendererName)
           ? ACTIVE_INTERFACE.metadata.rendererName
           : "No active renderer";
}
