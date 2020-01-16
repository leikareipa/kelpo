/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Direct3D 7 rasterizer for the Kelpo renderer.
 * 
 * Note: The DirectX 7 headers basically force the use of a C++ compiler -
 * hence, the code here might not conform to C89 like the rest of Kelpo.
 * 
 */

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <kelpo_interface/generic_stack.h>
#include <kelpo_renderer/surface/directdraw_7/create_directdraw_7_surface_from_texture.h>
#include <kelpo_renderer/surface/direct3d_7/surface_direct3d_7.h>
#include <kelpo_renderer/rasterizer/direct3d_7/rasterizer_direct3d_7.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/polygon/texture.h>

#include <windows.h>
#include <d3d.h>

/* For keeping track of where in texture memory textures have been uploaded.
 * Stack elements will be of type LPDIRECTDRAWSURFACE7, which represents a
 * pointer to the texture's DirectDraw 7 surface.*/
static struct kelpo_generic_stack_s *UPLOADED_TEXTURES;

extern LPDIRECT3DDEVICE7 D3DDEVICE_7;

void kelpo_rasterizer_direct3d_7__initialize(void)
{
    IDirect3DDevice7_SetTexture(D3DDEVICE_7, 0, NULL);
    
    IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
    IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0, D3DTSS_MAGFILTER, D3DTFN_LINEAR);

    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_ZENABLE, TRUE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_ZWRITEENABLE, TRUE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_CLIPPING, FALSE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_LIGHTING, FALSE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_AMBIENT, ~0u);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
    IDirect3DDevice7_SetRenderState(D3DDEVICE_7, D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);

    UPLOADED_TEXTURES = kelpo_generic_stack__create(10, sizeof(LPDIRECTDRAWSURFACE7));

    return;
}

void kelpo_rasterizer_direct3d_7__release(void)
{
    kelpo_generic_stack__free(UPLOADED_TEXTURES);

    return;
}

void kelpo_rasterizer_direct3d_7__clear_frame(void)
{
    IDirect3DDevice7_Clear(D3DDEVICE_7, 0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), 0, 1, 0);

    return;
}

void kelpo_rasterizer_direct3d_7__upload_texture(struct kelpo_polygon_texture_s *const texture)
{
    LPDIRECTDRAWSURFACE7 d3dTexture = kelpo_create_directdraw_7_surface_from_texture(texture, D3DDEVICE_7);

    assert(d3dTexture && "Direct3D 7: Failed to create a Direct3D texture.");

    texture->apiId = (uint32_t)d3dTexture;
    kelpo_generic_stack__push_copy(UPLOADED_TEXTURES, &d3dTexture);

    return;
}

void kelpo_rasterizer_direct3d_7__update_texture(struct kelpo_polygon_texture_s *const texture)
{
    /* TODO.*/
    
    return;
}

void kelpo_rasterizer_direct3d_7__purge_textures(void)
{
    unsigned i = 0;

    IDirect3DDevice7_SetTexture(D3DDEVICE_7, 0, NULL);

    for (i = 0; i < UPLOADED_TEXTURES->count; i++)
    {
        LPDIRECTDRAWSURFACE7 uploadedTexture = ((LPDIRECTDRAWSURFACE7*)UPLOADED_TEXTURES->data)[i];
        IDirectDrawSurface7_Release(uploadedTexture);
    }

    kelpo_generic_stack__clear(UPLOADED_TEXTURES);

    return;
}


void kelpo_rasterizer_direct3d_7__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
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
            verts[v].sz = triangles[i].vertex[v].z;
            verts[v].rhw = triangles[i].vertex[v].w;
            verts[v].tu = triangles[i].vertex[v].u;
            verts[v].tv = triangles[i].vertex[v].v;
            verts[v].color = RGBA_MAKE(triangles[i].vertex[v].r,
                                       triangles[i].vertex[v].g,
                                       triangles[i].vertex[v].b,
                                       triangles[i].vertex[v].a);
        }

        /* TODO: Reduce state-switching.*/

        if (triangles[i].texture)
        {
            const int mipmapEnabled = (triangles[i].texture->numMipLevels > 1);
            const int mipmapFilter = (triangles[i].texture->flags.noFiltering? D3DTFP_POINT : D3DTFP_LINEAR);

            IDirect3DDevice7_SetTexture(D3DDEVICE_7, 0, (LPDIRECTDRAWSURFACE7)(uint32_t)triangles[i].texture->apiId);

            IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0,
                                                  D3DTSS_MIPFILTER,
                                                  (mipmapEnabled? mipmapFilter : FALSE));

            IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0,
                                                  D3DTSS_MINFILTER,
                                                  (triangles[i].texture->flags.noFiltering? D3DTFN_POINT : D3DTFN_LINEAR));

            IDirect3DDevice7_SetTextureStageState(D3DDEVICE_7, 0,
                                                  D3DTSS_MAGFILTER,
                                                  (triangles[i].texture->flags.noFiltering? D3DTFN_POINT : D3DTFN_LINEAR));
        }

        IDirect3DDevice7_DrawPrimitive(D3DDEVICE_7,
                                       D3DPT_TRIANGLELIST,
                                       D3DFVF_TLVERTEX,
                                       verts, 3,
                                       NULL);

        IDirect3DDevice7_SetTexture(D3DDEVICE_7, 0, NULL);
    }

    IDirect3DDevice7_EndScene(D3DDEVICE_7);
    
    return;
}
