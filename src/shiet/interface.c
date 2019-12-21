#include <assert.h>
#include <string.h>
#include "shiet/interface.h"

#if _WIN32
    #include <windows.h>
    #define dll_function_address(dllName, functionName) (GetProcAddress(LoadLibraryA(dllName), functionName))
#else
    #error "Unknown platform."
#endif

typedef void *(*dll_init_t)(struct shiet_renderer_interface_s *const);

struct shiet_renderer_interface_s shiet_create_renderer_interface(const char *const rasterizerName)
{
    #define rasterizer_name_is(string) (strcmp(rasterizerName, string) == 0)

    dll_init_t set_interface_pointers = NULL;
    struct shiet_renderer_interface_s renderer = {NULL};

    if (rasterizer_name_is("OpenGL"))
    {
        set_interface_pointers = (dll_init_t)dll_function_address("shiet_renderer_opengl.dll", "shiet_renderer__set_function_pointers");
    }
    else
    {
        assert(0 && "Unrecognized rasterizer name.");
    }

    assert(set_interface_pointers && "Failed to fetch renderer library pointers.");
    set_interface_pointers(&renderer);

    #undef rasterizer_name_is

    renderer.metadata.shietMajorVersion = 0;
    renderer.metadata.shietMinorVersion = 0;
    renderer.metadata.shietPatchVersion = 1;

    return renderer;
}

#undef dll_address
