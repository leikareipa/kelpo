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

struct shiet_renderer_interface_s shiet_create_render_interface(const char *const rasterizer)
{
    #define rasterizer_name_is(name) (strcmp(rasterizer, "opengl") == 0)

    dll_init_t get_interface_pointers = NULL;
    struct shiet_renderer_interface_s intf = {NULL};

    if (rasterizer_name_is("opengl"))
    {
        get_interface_pointers = (dll_init_t)dll_function_address("shiet_renderer_opengl.dll", "shiet_renderer__get_function_pointers");
    }
    else
    {
        assert(0 && "Unrecognized rasterizer name.");
    }

    assert(get_interface_pointers && "Failed to fetch render library pointers.");
    get_interface_pointers(&intf);

    #undef rasterizer_name_is

    intf.metadata.shietMajorVersion = 0;
    intf.metadata.shietMinorVersion = 0;
    intf.metadata.shietPatchVersion = 1;

    return intf;
}

#undef dll_address
