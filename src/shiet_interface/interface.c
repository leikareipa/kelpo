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
    dll_init_t import_renderer = NULL;
    struct shiet_interface_s renderer = {NULL};

    if (strcmp("OpenGL", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("shiet_renderer_opengl_1_2.dll", "import_renderer");
    }
    else if (strcmp("Glide3", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("shiet_renderer_glide_3.dll", "import_renderer");
    }
    else if (strcmp("Direct3D7", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("shiet_renderer_direct3d_7.dll", "import_renderer");
    }
    else
    {
        assert(0 && "Unrecognized renderer.");
    }

    assert(import_renderer && "Failed to load the renderer library.");

    import_renderer(&renderer);

    assert((renderer.metadata.rendererVersionMajor == SHIET_INTERFACE_VERSION_MAJOR) &&
           "The renderer library's version is incompatible with this version of shiet.");

    return renderer;
}

#undef DLL_FUNC_ADDRESS
