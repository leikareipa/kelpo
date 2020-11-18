#include <assert.h>
#include <string.h>
#include "kelpo_interface/interface.h"

#if _WIN32
    #include <windows.h>
    #define DLL_FUNC_ADDRESS(dllName, functionName) (GetProcAddress(LoadLibraryA(dllName), functionName))
#else
    #error "Unknown platform."
#endif

typedef void *(*dll_init_t)(struct kelpo_interface_s *const);

struct kelpo_interface_s kelpo_create_interface(const char *const rasterizerName)
{
    const char *dllFilename = NULL;
    dll_init_t get_kelpo_interface = NULL;
    struct kelpo_interface_s kelpoInterface = {{0}};

         if (strcmp(rasterizerName, "opengl_1_2") == 0) dllFilename = "kelpo_renderer_opengl_1_2.dll";
    else if (strcmp(rasterizerName, "opengl_3_0") == 0) dllFilename = "kelpo_renderer_opengl_3_0.dll";
    else if (strcmp(rasterizerName, "glide_3")    == 0) dllFilename = "kelpo_renderer_glide_3.dll";
    else if (strcmp(rasterizerName, "direct3d_5") == 0) dllFilename = "kelpo_renderer_direct3d_5.dll";
    else if (strcmp(rasterizerName, "direct3d_6") == 0) dllFilename = "kelpo_renderer_direct3d_6.dll";
    else if (strcmp(rasterizerName, "direct3d_7") == 0) dllFilename = "kelpo_renderer_direct3d_7.dll";

    assert(dllFilename && "Unknown renderer name.");
    get_kelpo_interface = (dll_init_t)DLL_FUNC_ADDRESS(dllFilename, "export_interface");

    assert(get_kelpo_interface && "Could not import the renderer DLL.");
    get_kelpo_interface(&kelpoInterface);

    assert((kelpoInterface.metadata.rendererVersionMajor == KELPO_INTERFACE_VERSION_MAJOR) &&
           "The renderer's version is not compatible with this version of Kelpo.");

    return kelpoInterface;
}

#undef DLL_FUNC_ADDRESS
