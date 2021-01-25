/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * OpenGL 1.2 rasterizer for the Kelpo renderer.
 * 
 */

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <kelpo_renderer/rasterizer/opengl_1_2/rasterizer_opengl_1_2.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/polygon/texture.h>
#include <kelpo_interface/error.h>

#include <gl/gl.h>
#include <gl/glext.h>

/* For keeping track of where in texture memory textures have been uploaded.
 * Stack elements will be of type GLuint, which represents an OpenGL texture
 * ID as returned by glGenTextures().*/
static struct kelpoa_generic_stack_s *UPLOADED_TEXTURES;

void kelpo_rasterizer_opengl_1_2__initialize(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5);

    /* We'll generally provide our own mipmaps, so don't want OpenGL messing with them.*/
    #ifdef GL_GENERATE_MIPMAP
        glDisable(GL_GENERATE_MIPMAP);
    #endif

    UPLOADED_TEXTURES = kelpoa_generic_stack__create(10, sizeof(GLuint));

    return;
}

void kelpo_rasterizer_opengl_1_2__release(void)
{
    kelpoa_generic_stack__free(UPLOADED_TEXTURES);

    return;
}

void kelpo_rasterizer_opengl_1_2__clear_frame(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return;
}

static void set_parameters_for_texture(const struct kelpo_polygon_texture_s *const texture)
{
    assert((texture && texture->apiId) && "Invalid texture.");
    
    glBindTexture(GL_TEXTURE_2D, texture->apiId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (texture->flags.clamped? GL_CLAMP_TO_EDGE : GL_REPEAT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (texture->flags.clamped? GL_CLAMP_TO_EDGE : GL_REPEAT));

    if ((texture->numMipLevels <= 1) ||
        texture->flags.noMipmapping)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (texture->flags.noFiltering? GL_NEAREST : GL_LINEAR));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (texture->flags.noFiltering? GL_NEAREST : GL_LINEAR));
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (texture->flags.noFiltering? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (texture->flags.noFiltering? GL_NEAREST : GL_LINEAR));
    }

    return;
}

static void upload_texture_mipmap_data(const struct kelpo_polygon_texture_s *const texture)
{
    unsigned m = 0;

    assert((texture && texture->apiId) && "Invalid texture.");

    glBindTexture(GL_TEXTURE_2D, texture->apiId);

    for (m = 0; m < texture->numMipLevels; m++)
    {
        const unsigned mipLevelSideLength = (texture->width / pow(2, m));

        glTexSubImage2D(GL_TEXTURE_2D, m,
                        0, 0,
                        mipLevelSideLength, mipLevelSideLength,
                        GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV,
                        texture->mipLevel[m]);
    }

    return;
}

/* Uploads the given texture's data to the graphics device. An entry for this
 * texture must already have been created with glGenTextures() prior to calling
 * this function; the corresponding texture id must be stored in the texture's
 * 'apiId' property.*/
static void upload_texture_data(const struct kelpo_polygon_texture_s *const texture)
{
    uint32_t m = 0;
    const unsigned numMipLevels = (texture->flags.noMipmapping? 1 : texture->numMipLevels);

    assert(texture && "Attempting to process a NULL texture");

    glBindTexture(GL_TEXTURE_2D, texture->apiId);

    if (numMipLevels > 1)
    {
        for (m = 0; m < numMipLevels; m++)
        {
            const unsigned resDiv = (pow(2, m));
            glTexImage2D(GL_TEXTURE_2D, m, GL_RGBA, (texture->width / resDiv), (texture->height / resDiv), 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, texture->mipLevel[m]);
        }
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, texture->mipLevel[0]);
    }

    return;
}

void kelpo_rasterizer_opengl_1_2__upload_texture(struct kelpo_polygon_texture_s *const texture)
{
    assert(!glIsTexture(texture->apiId) &&
           "This texture has already been registered. Use update_texture() instead.");

    glGenTextures(1, (GLuint*)&texture->apiId);
    kelpoa_generic_stack__push_copy(UPLOADED_TEXTURES, &texture->apiId);

    upload_texture_data(texture);
    set_parameters_for_texture(texture);
    
    return;
}

void kelpo_rasterizer_opengl_1_2__update_texture(struct kelpo_polygon_texture_s *const texture)
{
    assert(texture && "Attempting to update a NULL texture");

    assert(glIsTexture(texture->apiId) &&
           "This texture has not yet been registered. Use upload_texture() instead.");

    /* TODO: Make sure the texture's dimensions and color depth haven't changed
     * since it was first uploaded.*/

    set_parameters_for_texture(texture);
    upload_texture_mipmap_data(texture);

    return;
}

void kelpo_rasterizer_opengl_1_2__unload_textures(void)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(UPLOADED_TEXTURES->count, UPLOADED_TEXTURES->data);
    kelpoa_generic_stack__clear(UPLOADED_TEXTURES);

    return;
}

void kelpo_rasterizer_opengl_1_2__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
                                                 const unsigned numTriangles)
{
    unsigned i = 0, v = 0;
    GLuint lastBoundTexture = 0; /* Assumes OpenGL never generates texture id 0.*/

    for (i = 0; i < numTriangles; i++)
    {
        if (!triangles[i].texture)
        {
            if (lastBoundTexture)
            {
                glDisable(GL_TEXTURE_2D);
            }
            
            lastBoundTexture = 0;

            glBegin(GL_TRIANGLES);
                for (v = 0; v < 3; v++)
                {
                    glColor3ub(triangles[i].vertex[v].r, triangles[i].vertex[v].g, triangles[i].vertex[v].b);
                    glVertex3f(triangles[i].vertex[v].x, -triangles[i].vertex[v].y, -triangles[i].vertex[v].z);
                }
            glEnd();
        }
        else
        {
            if (triangles[i].texture->apiId != lastBoundTexture)
            {
                lastBoundTexture = triangles[i].texture->apiId;

                glEnable(GL_TEXTURE_2D);
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
