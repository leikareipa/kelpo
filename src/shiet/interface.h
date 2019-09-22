#ifndef SHIET_INTERFACE_H_
#define SHIET_INTERFACE_H_

struct shiet_polygon_triangle_s;
struct shiet_polygon_texture_s;

struct shiet_renderer_interface_s
{
    void (*initialize)(const unsigned, const unsigned, const char *const);

    struct
    {
        void (*get_handle)(void**);
        void (*update_window)(void);
        int (*is_window_open)(void);
        /*size*/
    } window;

    struct
    {
        void (*clear_frame)(void);
        void (*upload_texture)(struct shiet_polygon_texture_s *const);
        void (*draw_triangles)(struct shiet_polygon_triangle_s *const, const unsigned);
        /*release*/
        /*framebuffer*/
        /*update_texture*/
    } rasterizer;

    struct
    {
        unsigned shietMajorVersion;
        unsigned shietMinorVersion;
        unsigned shietPatchVersion;

        const char *rendererName;
        unsigned rendererMajorVersion;
        unsigned rendererMinorVersion;
        unsigned rendererPatchVersion;
    } metadata;
};

struct shiet_renderer_interface_s shiet_create_render_interface(const char *const rasterizer);

#endif
