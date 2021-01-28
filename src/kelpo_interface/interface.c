#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <kelpo_interface/interface.h>
#include <kelpo_interface/error.h>

#include <windows.h>

typedef void *(*dll_import_fn_t)(struct kelpo_interface_s *const, const unsigned);

/* The current render interface. Created/initialized by the most recent call to
 * kelpo_create_interface(). Only one interface can be active at a time.*/
static struct kelpo_interface_s ACTIVE_INTERFACE = {0};

/* Initializes a Kelpo interface for the given renderer, and sets 'dst' to point
 * to that interface. Returns 1 on success; 0 on failure. If the call fails,
 * the dst pointer won't be modified.*/
int kelpo_create_interface(const struct kelpo_interface_s **dst,
                           const char *const rendererName)
{
    const char *dllFilename = NULL;
    dll_import_fn_t get_kelpo_interface = NULL;

    /* In case parts of the interface have already been initialized. We want a
     * clean slate.*/
    if (!kelpo_release_interface(&ACTIVE_INTERFACE))
    {
        /* kelpo_release_interface() is expected to have called kelpo_error(), so
         * we can just return.*/
        return 0;
    }

    /* For debugging purposes, record the user-provided renderer name. If the
     * interface is successfully created, this will be replaced with the actual
     * renderer name.*/
    KELPO_COPY_RENDERER_NAME(ACTIVE_INTERFACE.metadata.rendererName, rendererName);

    if      (strcmp(rendererName, "opengl_1_2") == 0) dllFilename = "kelpo_renderer_opengl_1_2.dll";
    else if (strcmp(rendererName, "opengl_3_0") == 0) dllFilename = "kelpo_renderer_opengl_3_0.dll";
    else if (strcmp(rendererName, "glide_3")    == 0) dllFilename = "kelpo_renderer_glide_3.dll";
    else if (strcmp(rendererName, "direct3d_5") == 0) dllFilename = "kelpo_renderer_direct3d_5.dll";
    else if (strcmp(rendererName, "direct3d_6") == 0) dllFilename = "kelpo_renderer_direct3d_6.dll";
    else if (strcmp(rendererName, "direct3d_7") == 0) dllFilename = "kelpo_renderer_direct3d_7.dll";

    if (!dllFilename ||
        !(ACTIVE_INTERFACE.dllHandle = LoadLibraryA(dllFilename)))
    {
        kelpo_error(KELPOERR_RENDERER_NOT_AVAILABLE);
        return 0;
    }

    get_kelpo_interface = (dll_import_fn_t)GetProcAddress(ACTIVE_INTERFACE.dllHandle, "export_interface");
    assert(get_kelpo_interface && "Malformed renderer DLL; required export not found.");

    if (!get_kelpo_interface(&ACTIVE_INTERFACE, KELPO_INTERFACE_VERSION_MAJOR))
    {
        kelpo_error(KELPOERR_RENDERER_NOT_COMPATIBLE_WITH_INTERFACE);
        return 0;
    }

    *dst = &ACTIVE_INTERFACE;
    return 1;
}

/* Deallocates the given interface. The interface can no longer be used after
 * calling this function. Returns 1 on success; 0 on failure. If the call succeeds,
 * the interface's function pointers and data will be reset to NULL values;
 * otherwise, the values won't be modified.*/
int kelpo_release_interface(const struct kelpo_interface_s *const kelpoInterface)
{
    /* The given interface pointer could be NULL e.g. if it was initialized that
     * way and a call to kelpo_create_interface() failed. So we allow it and just
     * silently return.*/
    if (!kelpoInterface)
    {
        return 1;
    }

    assert((kelpoInterface == &ACTIVE_INTERFACE) && 
           "Can't release an interface that isn't active.");

    /* Since opening the Kelpo renderer window acts as a renderer initializer,
     * we assume that if the window isn't open, the renderer failed to initialize;
     * in which case we shoudn't attempt to call any releasing functions on it.*/
    if (ACTIVE_INTERFACE.window.is_open &&
        ACTIVE_INTERFACE.window.is_open())
    {
        if (ACTIVE_INTERFACE.rasterizer.unload_textures &&
            !ACTIVE_INTERFACE.rasterizer.unload_textures())
        {
            /* The function call is expected to have called kelpo_error(), so we
            * can just return.*/
            return 0;
        }

        if (ACTIVE_INTERFACE.window.release &&
            !ACTIVE_INTERFACE.window.release())
        {
            /* The function call is expected to have called kelpo_error(), so we
            * can just return.*/
            return 0;
        }
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
