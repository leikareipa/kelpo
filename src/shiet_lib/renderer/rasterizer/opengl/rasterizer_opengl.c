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
#include <shiet_lib/renderer/rasterizer/opengl/rasterizer_opengl.h>
#include <shiet/polygon/triangle/triangle.h>
#include <shiet/polygon/texture.h>

#include <gl/gl.h>
#include <gl/glext.h>

void shiet_rasterizer_opengl__initialize(void)
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

void shiet_rasterizer_opengl__clear_frame(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return;
}

void shiet_rasterizer_opengl__upload_texture(struct shiet_polygon_texture_s *const texture)
{
    uint32_t m = 0;

    if (!texture)
    {
        return;
    }

    assert(!glIsTexture(texture->apiId) &&
           "Attempting to upload a texture that has already been uploaded.");

    glGenTextures(1, (GLuint*)&texture->apiId);
    glBindTexture(GL_TEXTURE_2D, texture->apiId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (texture->numMipLevels > 1)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ((texture->filtering == SHIET_TEXTURE_FILTER_LINEAR)? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ((texture->filtering == SHIET_TEXTURE_FILTER_LINEAR)? GL_LINEAR : GL_NEAREST));

        for (m = 0; m < texture->numMipLevels; m++)
        {
            const unsigned resDiv = (pow(2, m));
            glTexImage2D(GL_TEXTURE_2D, m, GL_RGBA, (texture->width / resDiv), (texture->height / resDiv), 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, texture->mipLevel[m]);
        }
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ((texture->filtering == SHIET_TEXTURE_FILTER_LINEAR)? GL_LINEAR : GL_NEAREST));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ((texture->filtering == SHIET_TEXTURE_FILTER_LINEAR)? GL_LINEAR : GL_NEAREST));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, texture->mipLevel[0]);
    }
    
    return;
}

void shiet_rasterizer_opengl__update_texture(const struct shiet_polygon_texture_s *const texture)
{
    uint32_t m = 0;

    if (!texture ||
        !glIsTexture(texture->apiId))
    {
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, texture->apiId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (texture->numMipLevels > 1)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ((texture->filtering == SHIET_TEXTURE_FILTER_LINEAR)? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ((texture->filtering == SHIET_TEXTURE_FILTER_LINEAR)? GL_LINEAR : GL_NEAREST));

        for (m = 0; m < texture->numMipLevels; m++)
        {
            const unsigned resDiv = (pow(2, m));
            glTexImage2D(GL_TEXTURE_2D, m, GL_RGBA, (texture->width / resDiv), (texture->height / resDiv), 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, texture->mipLevel[m]);
        }
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ((texture->filtering == SHIET_TEXTURE_FILTER_LINEAR)? GL_LINEAR : GL_NEAREST));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ((texture->filtering == SHIET_TEXTURE_FILTER_LINEAR)? GL_LINEAR : GL_NEAREST));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, texture->mipLevel[0]);
    }

    return;
}

void shiet_rasterizer_opengl__draw_triangles(const struct shiet_polygon_triangle_s *const triangles,
                                             const unsigned numTriangles)
{
    unsigned i = 0;

    for (i = 0; i < numTriangles; i++)
    {
        const struct shiet_polygon_texture_s *const texture = triangles[i].material.texture;

        if (texture == NULL)
        {
            glDisable(GL_TEXTURE_2D);
            glColor4ub(triangles[i].material.baseColor[0],
                       triangles[i].material.baseColor[1],
                       triangles[i].material.baseColor[2],
                       triangles[i].material.baseColor[3]);

            glBegin(GL_TRIANGLES);
                glVertex2f(triangles[i].vertex[0].x, -triangles[i].vertex[0].y);
                glVertex2f(triangles[i].vertex[1].x, -triangles[i].vertex[1].y);
                glVertex2f(triangles[i].vertex[2].x, -triangles[i].vertex[2].y);
            glEnd();
        }
        else
        {
            unsigned v = 0;

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture->apiId);
            glColor4ub(255, 255, 255, 255);

            glBegin(GL_TRIANGLES);
                for (v = 0; v < 3; v++)
                {
                    glTexCoord4f((triangles[i].vertex[v].u / triangles[i].vertex[v].w),
                                 (triangles[i].vertex[v].v / triangles[i].vertex[v].w),
                                 0, (1 / triangles[i].vertex[v].w));
                    glNormal3f(triangles[i].vertex[v].nx, triangles[i].vertex[v].ny, triangles[i].vertex[v].nz);
                    glVertex3f(triangles[i].vertex[v].x, -triangles[i].vertex[v].y, -triangles[i].vertex[v].z);
                }
            glEnd();
        }
    }

    return;
}
