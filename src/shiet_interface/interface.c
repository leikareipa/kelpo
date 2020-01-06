#include <assert.h>
#include <string.h>
#include "shiet_interface/interface.h"

#if _WIN32
    #include <windows.h>
    #define DLL_FUNC_ADDRESS(dllName, functionName) (GetProcAddress(LoadLibraryA(dllName), functionName))
#else
    #error "Unknown platform."
#endif

typedef void *(*dll_init_t)(struct shiet_interface_s *const);

struct shiet_interface_s shiet_create_interface(const char *const rasterizerName)
{
    dll_init_t set_interface_pointers = NULL;
    struct shiet_interface_s renderer = {NULL};

    if (strcmp("OpenGL", rasterizerName) == 0)
    {
        set_interface_pointers = (dll_init_t)DLL_FUNC_ADDRESS("shiet_renderer_opengl.dll",
                                                              "shiet_renderer_opengl__set_function_pointers");
    }
    else if (strcmp("Glide3", rasterizerName) == 0)
    {
        set_interface_pointers = (dll_init_t)DLL_FUNC_ADDRESS("shiet_renderer_glide3.dll",
                                                              "shiet_renderer_glide3__set_function_pointers");
    }
    else
    {
        assert(0 && "Unrecognized renderer.");
    }

    assert(set_interface_pointers && "Failed to load the renderer library.");

    set_interface_pointers(&renderer);

    return renderer;
}

#undef DLL_FUNC_ADDRESS
