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
    dll_init_t import_renderer = NULL;
    struct kelpo_interface_s renderer = {{0}};

    if (strcmp("opengl_1_2", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("kelpo_renderer_opengl_1_2.dll", "import_renderer");
    }
    else if (strcmp("opengl_3_0", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("kelpo_renderer_opengl_3_0.dll", "import_renderer");
    }
    else if (strcmp("glide_3", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("kelpo_renderer_glide_3.dll", "import_renderer");
    }
    else if (strcmp("direct3d_5", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("kelpo_renderer_direct3d_5.dll", "import_renderer");
    }
    else if (strcmp("direct3d_6", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("kelpo_renderer_direct3d_6.dll", "import_renderer");
    }
    else if (strcmp("direct3d_7", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("kelpo_renderer_direct3d_7.dll", "import_renderer");
    }
    else if (strcmp("software_directdraw_7", rasterizerName) == 0)
    {
        import_renderer = (dll_init_t)DLL_FUNC_ADDRESS("kelpo_renderer_software_directdraw_7.dll", "import_renderer");
    }
    else
    {
        assert(0 && "Unrecognized renderer.");
    }

    assert(import_renderer && "Failed to load the renderer library.");

    import_renderer(&renderer);

    assert((renderer.metadata.rendererVersionMajor == KELPO_INTERFACE_VERSION_MAJOR) &&
           "The renderer library's version is incompatible with this version of Kelpo.");

    return renderer;
}

#undef DLL_FUNC_ADDRESS
