/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * OpenGL 1.2 rasterizer for the shiet renderer.
 * 
 */

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <shiet_lib/renderer/rasterizer/opengl_1_2/rasterizer_opengl_1_2.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include <shiet_interface/polygon/texture.h>

#include <gl/gl.h>
#include <gl/glext.h>

void shiet_rasterizer_opengl_1_2__initialize(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    /* We'll generally provide our own mipmaps, so don't want OpenGL messing with them.*/
    #ifdef GL_GENERATE_MIPMAP
        glDisable(GL_GENERATE_MIPMAP);
    #endif

    return;
}

void shiet_rasterizer_opengl_1_2__clear_frame(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return;
}

/* Uploads the given texture's data to the graphics device. An entry for this
 * texture must already have been created with glGenTextures() prior to calling
 * this function; the corresponding texture id must be stored in the texture's
 * 'apiId' property.*/
static void upload_texture_data(struct shiet_polygon_texture_s *const texture)
{
    uint32_t m = 0;

    assert(texture &&
           "OpenGL 1.2 renderer: Attempting to process a NULL texture");

    glBindTexture(GL_TEXTURE_2D, texture->apiId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (texture->flags.clamped? GL_CLAMP_TO_EDGE : GL_REPEAT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (texture->flags.clamped? GL_CLAMP_TO_EDGE : GL_REPEAT));

    if (texture->numMipLevels > 1)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (texture->flags.noFiltering? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (texture->flags.noFiltering? GL_NEAREST : GL_LINEAR));

        for (m = 0; m < texture->numMipLevels; m++)
        {
            const unsigned resDiv = (pow(2, m));
            glTexImage2D(GL_TEXTURE_2D, m, GL_RGBA, (texture->width / resDiv), (texture->height / resDiv), 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, texture->mipLevel[m]);
        }
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (texture->flags.noFiltering? GL_NEAREST : GL_LINEAR));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (texture->flags.noFiltering? GL_NEAREST : GL_LINEAR));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, texture->mipLevel[0]);
    }

    return;
}

void shiet_rasterizer_opengl_1_2__upload_texture(struct shiet_polygon_texture_s *const texture)
{
    assert(!glIsTexture(texture->apiId) &&
           "OpenGL 1.2 renderer: This texture has already been registered. Use update_texture() instead.");

    glGenTextures(1, (GLuint*)&texture->apiId);

    upload_texture_data(texture);
    
    return;
}

void shiet_rasterizer_opengl_1_2__update_texture(struct shiet_polygon_texture_s *const texture)
{
    assert(texture &&
           "OpenGL 1.2 renderer: Attempting to update a NULL texture");

    assert(glIsTexture(texture->apiId) &&
           "OpenGL 1.2 renderer: This texture has not yet been registered. Use upload_texture() instead.");

    upload_texture_data(texture);

    return;
}

void shiet_rasterizer_opengl_1_2__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                             const unsigned numTriangles)
{
    unsigned i = 0, v = 0;
    GLuint lastBoundTexture = 0; /* Assumes OpenGL never generates texture id 0.*/

    for (i = 0; i < numTriangles; i++)
    {
        if (!triangles[i].texture)
        {
            glDisable(GL_TEXTURE_2D);
            
            glBegin(GL_TRIANGLES);
                for (v = 0; v < 3; v++)
                {
                    glColor3ub(triangles[i].vertex[v].r, triangles[i].vertex[v].g, triangles[i].vertex[v].b);
                    glVertex2f(triangles[i].vertex[v].x, -triangles[i].vertex[v].y);
                }
            glEnd();
        }
        else
        {
            glEnable(GL_TEXTURE_2D);

            if (triangles[i].texture->apiId != lastBoundTexture)
            {
                glBindTexture(GL_TEXTURE_2D, triangles[i].texture->apiId);
            }

            glBegin(GL_TRIANGLES);
                for (v = 0; v < 3; v++)
                {
                    glTexCoord4f((triangles[i].vertex[v].u * triangles[i].vertex[v].w),
                                 (triangles[i].vertex[v].v * triangles[i].vertex[v].w),
                                 0, triangles[i].vertex[v].w);
                    glNormal3f(triangles[i].vertex[v].nx, triangles[i].vertex[v].ny, triangles[i].vertex[v].nz);
                    glColor4ub(triangles[i].vertex[v].r, triangles[i].vertex[v].g, triangles[i].vertex[v].b, triangles[i].vertex[v].a);
                    glVertex3f(triangles[i].vertex[v].x, -triangles[i].vertex[v].y, -triangles[i].vertex[v].z);
                }
            glEnd();
        }
    }

    return;
}
