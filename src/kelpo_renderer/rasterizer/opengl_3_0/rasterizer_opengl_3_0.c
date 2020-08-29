/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * OpenGL 3.0 rasterizer for the Kelpo renderer.
 * 
 */

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <kelpo_renderer/rasterizer/opengl_3_0/rasterizer_opengl_3_0.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/polygon/texture.h>

#include <gl/gl.h>
#include <kelpo_renderer/surface/opengl_3_0/gl3_types.h>

/* For keeping track of where in texture memory textures have been uploaded.
 * Stack elements will be of type GLuint, which represents an OpenGL texture
 * ID as returned by glGenTextures().*/
static struct kelpoa_generic_stack_s *UPLOADED_TEXTURES;

struct gl3_vertex_s
{
    float x, y, z, w;
    float u, v;
    float r, g, b, a;
};

void kelpo_rasterizer_opengl_3_0__initialize(void)
{
    GLuint shaderProgram = glCreateProgram();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0);

    /* We'll generally provide our own mipmaps, so don't want OpenGL messing with them.*/
    #ifdef GL_GENERATE_MIPMAP
        glDisable(GL_GENERATE_MIPMAP);
    #endif

    UPLOADED_TEXTURES = kelpoa_generic_stack__create(10, sizeof(GLuint));

    /* Compile the vertex and fragment shaders.*/
    {
        const char *const vertexShaderSrc =
            "#version 130\n"

            "in vec4 position;\n"
            "in vec4 color;\n"
            "in vec2 uv;\n"

            "out vec4 vertexColor;\n"
            "out vec4 vertexSTPQ;\n"

            "void main()\n"
            "{\n"
                "// UV coordinates for perspective-correct texture-mapping.\n"
                "vertexSTPQ = vec4((uv * position.w), 0, position.w);\n"

                "vertexColor = color;\n"

                "gl_Position = gl_ModelViewProjectionMatrix * vec4(position.xyz, 1);\n"
            "}";

        const char *const fragmentShaderSrc = 
            "#version 130\n"

            "uniform sampler2D texture;\n"

            "in vec4 vertexColor;\n"
            "in vec4 vertexSTPQ;\n"

            "void main()\n"
            "{\n"
                "// Perspective-correct texture-mapping.\n"
                "gl_FragColor = vertexColor * texture2D(texture, (vertexSTPQ.st / vertexSTPQ.q));\n"
            "}";

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        GLint status;

        glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
        assert(status && "OpenGL 3.0: Vertex shader compilation failed.");

        glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
        assert(status && "OpenGL 3.0: fragment shader compilation failed.");

        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);
    }

    /* Create vertex objects.*/
    {
        GLuint vbo;
        GLuint vao;

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    }

    /* Establish shader arguments. Data byte offsets are for the gl3_vertex_s
     * struct.*/
    {
        GLint position = glGetAttribLocation(shaderProgram, "position");
        GLint uv = glGetAttribLocation(shaderProgram, "uv");
        GLint color = glGetAttribLocation(shaderProgram, "color"); 

        glVertexAttribPointer(position, 4, GL_FLOAT, GL_FALSE, sizeof(struct gl3_vertex_s), (void*)0);
        glEnableVertexAttribArray(position);

        glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, sizeof(struct gl3_vertex_s), (void*)16);
        glEnableVertexAttribArray(uv);

        glVertexAttribPointer(color, 4, GL_FLOAT, GL_FALSE, sizeof(struct gl3_vertex_s), (void*)24);
        glEnableVertexAttribArray(color);
    }

    return;
}

void kelpo_rasterizer_opengl_3_0__release(void)
{
    kelpoa_generic_stack__free(UPLOADED_TEXTURES);

    return;
}

void kelpo_rasterizer_opengl_3_0__clear_frame(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return;
}

/* Uploads the given texture's data to the graphics device. An entry for this
 * texture must already have been created with glGenTextures() prior to calling
 * this function; the corresponding texture id must be stored in the texture's
 * 'apiId' property.*/
static void upload_texture_data(struct kelpo_polygon_texture_s *const texture)
{
    uint32_t m = 0;

    assert(texture &&
           "OpenGL 3.0: Attempting to process a NULL texture");

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

void kelpo_rasterizer_opengl_3_0__upload_texture(struct kelpo_polygon_texture_s *const texture)
{
    assert(!glIsTexture(texture->apiId) &&
           "OpenGL 3.0: This texture has already been registered. Use update_texture() instead.");

    glGenTextures(1, (GLuint*)&texture->apiId);
    kelpoa_generic_stack__push_copy(UPLOADED_TEXTURES, &texture->apiId);

    upload_texture_data(texture);
    
    return;
}

void kelpo_rasterizer_opengl_3_0__update_texture(struct kelpo_polygon_texture_s *const texture)
{
    assert(texture &&
           "OpenGL 3.0: Attempting to update a NULL texture");

    assert(glIsTexture(texture->apiId) &&
           "OpenGL 3.0: This texture has not yet been registered. Use upload_texture() instead.");

    /* TODO: Make sure the texture's dimensions and color depth haven't changed
     * since it was first uploaded.*/

    /* Copy each mip level's pixel data into the OpenGL texture.*/
    {
        unsigned m = 0;
        
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
    }

    return;
}

void kelpo_rasterizer_opengl_3_0__purge_textures(void)
{
    glDeleteTextures(UPLOADED_TEXTURES->count, UPLOADED_TEXTURES->data);
    kelpoa_generic_stack__clear(UPLOADED_TEXTURES);

    return;
}

void kelpo_rasterizer_opengl_3_0__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
                                                 const unsigned numTriangles)
{
    unsigned i = 0;
    GLuint lastBoundTexture = 0; /* Assumes OpenGL never generates texture id 0.*/

    for (i = 0; i < numTriangles; i++)
    {
        unsigned v = 0;
        struct gl3_vertex_s verts[3];

        if (!triangles[i].texture)
        {
            glDisable(GL_TEXTURE_2D);
        }
        else
        {
            glEnable(GL_TEXTURE_2D);

            if (triangles[i].texture->apiId != lastBoundTexture)
            {
                glBindTexture(GL_TEXTURE_2D, triangles[i].texture->apiId);
            }
        }

        for (v = 0; v < 3; v++)
        {
            const struct kelpo_polygon_vertex_s *srcVertex = &triangles[i].vertex[v];

            verts[v].x = srcVertex->x;
            verts[v].y = -srcVertex->y;
            verts[v].z = -srcVertex->z;
            verts[v].w = srcVertex->w;
            verts[v].u = srcVertex->u;
            verts[v].v = srcVertex->v;
            verts[v].r = (srcVertex->r / 255.0);
            verts[v].g = (srcVertex->g / 255.0);
            verts[v].b = (srcVertex->b / 255.0);
            verts[v].a = (srcVertex->a / 255.0);
        }

        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(verts), verts, GL_STATIC_DRAW);
        
        glDrawArrays(GL_TRIANGLES, 0, (numTriangles * 3));
    }

    return;
}
