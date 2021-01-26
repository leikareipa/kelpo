# Kelpo

More info to come.

# Developer's manual

## Examples

You can find small, contained samples of Kelpo in use under the [examples/](./examples/) directory.

It may be best to start by looking at the `simple_triangle` example. It does nothing but render a single static triangle, and so strips away everything except the bare essentials needed to incorporate Kelpo into an application.

## Code organization

You can find the full source code to Kelpo in the [src/](./src/) directory.

The Kelpo codebase is split into three categories:

1. Renderer ([src/kelpo_renderer/](./src/kelpo_renderer/))
2. Interface ([src/kelpo_interface/](./src/kelpo_interface/))
3. Auxiliary ([src/kelpo_auxiliary/](./src/kelpo_auxiliary/))

### Renderer

Renderer code is found under [src/kelpo_renderer/](./src/kelpo_renderer/).

A renderer represents a 3D API wrapped by Kelpo; e.g. Direct3D 7. It includes rasterizing and window management functionality using that API, and compiles into a standalone DLL library to be bundled with a client application (an application that uses Kelpo).

All Kelpo renderers provide the function `export_interface()`, defined in the corresponding `renderer_xxxx.c` source file found under [src/kelpo_renderer/](./src/kelpo_renderer/), which exports the renderer's code into a client application (see [interface](#interface)).

Client applications don't include renderer code directly, only the compiled renderer libraries.

Public functions and variables in renderer code use the `kelpo_` prefix.

#### Sub-categories

Renderer code is further split into three sub-categories:

1. Rasterizer
2. Window
3. Surface

Rasterizer code is responsible for rendering polygons using rasterizing functionality provided by the API (e.g. OpenGL 1.2). Public functions in rasterizer code use the `kelpo_rasterizer_xxxx__` prefix, where "xxxx" is the API's name (e.g. "opengl_1_2").

Window code provides the means to create and manage a Win32 window in which rasterized images can be displayed to the user. Public functions in window code use the `kelpo_window__` prefix.

Surface code mediates between the rasterizer and the window, providing the rasterizer a surface to render into and inserting the surface into the window for display. (Some renderers rely on a combination of surfaces: for instance, the Direct3D 5 API requires by its design both a Direct3D and a DirectDraw surface.) Public functions in surface code use the `kelpo_surface_xxxx__` prefix, where "xxxx" is the API's name (e.g. "glide_3").

### Interface

Interface code is found under [src/kelpo_interface/](./src/kelpo_interface/).

Client applications include and use the interface code to interact with Kelpo's renderer libraries.

Interface code provides the `kelpo_interface_s` struct populated via `kelpo_create_interface()` with functions that allow a client application to operate a given Kelpo renderer.

Public functions and variables in interface code use the `kelpo_` prefix.

### Auxiliary

Auxiliary code is found under [src/kelpo_auxiliary/](./src/kelpo_auxiliary/).

Auxiliary code provides Kelpo-compatible implementations of various rendering-related tasks, e.g. triangle transformation. Client applications can optionally include some or all of this functionality to avoid having to implement it themselves.

Kelpo's auxiliary code is neither required by Kelpo to operate nor guaranteed to be best-case implementations. You can use it as a time saver or a starting point, or choose to bring your own - it's up to you.

Public functions and variables in auxiliary code use the `kelpoa_` prefix.

# Screenshots

![](./images/screenshots/alpha/tr1-3b_glide3x_win98_voodoo1.png)\
**Scene:** Tomb Raider 1, level 4\
**Kelpo version:** alpha\
**Renderer:** Glide 3\
**GPU:** 3dfx Voodoo\
**CPU:** AMD K6 300\
**OS:** Windows 98

![](./images/screenshots/alpha/tr1-3b_d3d5_wine-linux_gtx-980.png)\
**Scene:** Tomb Raider 1, level 4\
**Kelpo version:** alpha\
**Renderer:** Direct3D 5\
**GPU:** GeForce GTX 980\
**OS:** Wine/Linux
