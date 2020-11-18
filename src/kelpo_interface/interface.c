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
    dll_init_t initialize_renderer = NULL;
    struct kelpo_interface_s renderer = {{0}};

         if (strcmp(rasterizerName, "opengl_1_2") == 0) dllFilename = "kelpo_renderer_opengl_1_2.dll";
    else if (strcmp(rasterizerName, "opengl_3_0") == 0) dllFilename = "kelpo_renderer_opengl_3_0.dll";
    else if (strcmp(rasterizerName, "glide_3")    == 0) dllFilename = "kelpo_renderer_glide_3.dll";
    else if (strcmp(rasterizerName, "direct3d_5") == 0) dllFilename = "kelpo_renderer_direct3d_5.dll";
    else if (strcmp(rasterizerName, "direct3d_6") == 0) dllFilename = "kelpo_renderer_direct3d_6.dll";
    else if (strcmp(rasterizerName, "direct3d_7") == 0) dllFilename = "kelpo_renderer_direct3d_7.dll";

    assert(dllFilename && "Invalid renderer name.");
    initialize_renderer = (dll_init_t)DLL_FUNC_ADDRESS(dllFilename, "import_renderer");

    assert(initialize_renderer && "Could not import the renderer DLL.");
    initialize_renderer(&renderer);

    assert((renderer.metadata.rendererVersionMajor == KELPO_INTERFACE_VERSION_MAJOR) &&
           "The renderer's version is not compatible with this version of Kelpo.");

    return renderer;
}

#undef DLL_FUNC_ADDRESS
