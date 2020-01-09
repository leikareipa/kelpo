/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Direct3D 7 rasterizer for the shiet renderer.
 * 
 */

#include <string.h>
#include <stdio.h>
#include <shiet_renderer/rasterizer/direct3d_7/surface_direct3d_7_win32.h>
#include <shiet_renderer/rasterizer/direct3d_7/rasterizer_direct3d_7.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include <shiet_interface/polygon/texture.h>

#include <windows.h>
#include <d3d.h>

extern LPDIRECT3DDEVICE7 D3DDEVICE_7;

void shiet_rasterizer_direct3d_7__initialize(void)
{
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_CLIPPING, FALSE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_LIGHTING, FALSE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_AMBIENT, ~0u);

    return;
}

void shiet_rasterizer_direct3d_7__clear_frame(void)
{
    IDirect3DDevice7_Clear(D3DDEVICE_7, 0, NULL, D3DCLEAR_TARGET, 0, 0, 0);

    return;
}

void shiet_rasterizer_direct3d_7__upload_texture(struct shiet_polygon_texture_s *const texture)
{
    /* TODO.*/

    return;
}

void shiet_rasterizer_direct3d_7__update_texture(struct shiet_polygon_texture_s *const texture)
{
    /* TODO.*/

    return;
}

void shiet_rasterizer_direct3d_7__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                                 const unsigned numTriangles)
{
    unsigned i = 0, v = 0;

    if (FAILED(IDirect3DDevice7_BeginScene(D3DDEVICE_7)))
    {
        return;
    }

    for (i = 0; i < numTriangles; i++)
    {
        D3DTLVERTEX verts[3];

        for (v = 0; v < 3; v++)
        {
            verts[v].sx = triangles[i].vertex[v].x;
            verts[v].sy = triangles[i].vertex[v].y;
            verts[v].rhw = triangles[i].vertex[v].w;
            verts[v].color = RGBA_MAKE(triangles[i].vertex[v].r,
                                       triangles[i].vertex[v].g,
                                       triangles[i].vertex[v].b,
                                       triangles[i].vertex[v].a);
        }

        IDirect3DDevice7_DrawPrimitive(D3DDEVICE_7,
                                       D3DPT_TRIANGLELIST,
                                       D3DFVF_TLVERTEX,
                                       verts, 3,
                                       NULL);
    }

    IDirect3DDevice7_EndScene(D3DDEVICE_7);
    
    return;
}
