/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * OpenGL 3.0 rasterizer for the Kelpo renderer.
 * 
 */

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <kelpo_renderer/rasterizer/opengl_3_0/rasterizer_opengl_3_0.h>
#include <kelpo_auxiliary/generic_stack.h>
#include <kelpo_interface/polygon/triangle/triangle.h>
#include <kelpo_interface/polygon/texture.h>
#include <kelpo_interface/error.h>

#include <gl/gl.h>
#include <kelpo_renderer/surface/opengl_3_0/gl3_types.h>

/* For keeping track of where in texture memory textures have been uploaded.
 * Stack elements will be of type GLuint, which represents an OpenGL texture
 * ID as returned by glGenTextures().*/
static struct kelpoa_generic_stack_s *UPLOADED_TEXTURES;

/* For temporary storage of vertices during rendering. Stack elements will be
 * of type struct gl3_vertex_s.*/
static struct kelpoa_generic_stack_s *GL3_VERTEX_CACHE;

struct gl3_vertex_s
{
    float x, y, z, w;
    float u, v;
    float r, g, b, a;
};

int kelpo_rasterizer_opengl_3_0__initialize(void)
{
    GLuint shaderProgram = glCreateProgram();

    UPLOADED_TEXTURES = kelpoa_generic_stack__create(10, sizeof(GLuint));
    GL3_VERTEX_CACHE = kelpoa_generic_stack__create(1000, sizeof(struct gl3_vertex_s));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.5);

    /* We'll generally provide our own mipmaps, so don't want OpenGL messing with them.*/
    #ifdef GL_GENERATE_MIPMAP
        glDisable(GL_GENERATE_MIPMAP);
    #endif

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
        assert(status && "Vertex shader compilation failed.");

        glShaderSource(fragmentShader, 1, &fragmentShaderSrc, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
        assert(status && "fragment shader compilation failed.");

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

    return 1;
}

int kelpo_rasterizer_opengl_3_0__release(void)
{
    kelpoa_generic_stack__free(UPLOADED_TEXTURES);
    kelpoa_generic_stack__free(GL3_VERTEX_CACHE);

    return 1;
}

int kelpo_rasterizer_opengl_3_0__clear_frame(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return 1;
}

static int set_parameters_for_texture(const struct kelpo_polygon_texture_s *const texture)
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

    return 1;
}

static int upload_texture_mipmap_data(const struct kelpo_polygon_texture_s *const texture)
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

    return 1;
}

/* Uploads the given texture's data to the graphics device. An entry for this
 * texture must already have been created with glGenTextures() prior to calling
 * this function; the corresponding texture id must be stored in the texture's
 * 'apiId' property.*/
static int upload_texture_data(struct kelpo_polygon_texture_s *const texture)
{
    uint32_t m = 0;

    assert(texture && "Attempting to process a NULL texture");

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

    return 1;
}

int kelpo_rasterizer_opengl_3_0__upload_texture(struct kelpo_polygon_texture_s *const texture)
{
    assert(!glIsTexture(texture->apiId) &&
           "This texture has already been registered. Use update_texture() instead.");

    glGenTextures(1, (GLuint*)&texture->apiId);
    kelpoa_generic_stack__push_copy(UPLOADED_TEXTURES, &texture->apiId);

    if (!upload_texture_data(texture) ||
        !set_parameters_for_texture(texture))
    {
        return 0;
    }
    
    return 1;
}

int kelpo_rasterizer_opengl_3_0__update_texture(struct kelpo_polygon_texture_s *const texture)
{
    assert(texture && "Attempting to update a NULL texture");

    assert(glIsTexture(texture->apiId) &&
           "This texture has not yet been registered. Use upload_texture() instead.");

    /* TODO: Make sure the texture's dimensions and color depth haven't changed
     * since it was first uploaded.*/

    if (!set_parameters_for_texture(texture) ||
        !upload_texture_mipmap_data(texture))
    {
        return 0;
    }

    return 1;
}

int kelpo_rasterizer_opengl_3_0__unload_textures(void)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(UPLOADED_TEXTURES->count, UPLOADED_TEXTURES->data);
    kelpoa_generic_stack__clear(UPLOADED_TEXTURES);

    return 1;
}

int kelpo_rasterizer_opengl_3_0__draw_triangles(struct kelpo_polygon_triangle_s *const triangles,
                                                const unsigned numTriangles)
{
    unsigned numTrianglesProcessed = 0;
    unsigned numTrianglesInBatch = 0;
    const struct kelpo_polygon_triangle_s *triangle = triangles;

    if (!numTriangles)
    {
        return 1;
    }

    if ((3 * numTriangles) > GL3_VERTEX_CACHE->capacity)
    {
        kelpoa_generic_stack__grow(GL3_VERTEX_CACHE, (3 * numTriangles));
    }

    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(struct gl3_vertex_s) * 3 * numTriangles), NULL, GL_DYNAMIC_DRAW);

    /* Render the triangles in batches. Each batch consists of consecutive
     * triangles that share a texture.*/
    while (1)
    {
        struct gl3_vertex_s *const vertexCache = (struct gl3_vertex_s*)GL3_VERTEX_CACHE->data;
        
        /* Add the current triangle into the batch.*/
        {
            unsigned v = 0;

            for (v = 0; v < 3; v++)
            {
                const struct kelpo_polygon_vertex_s *const srcVertex = &triangle->vertex[v];
                struct gl3_vertex_s *const dstVertex = &vertexCache[(numTrianglesInBatch * 3) + v];

                dstVertex->x = srcVertex->x;
                dstVertex->y = -srcVertex->y;
                dstVertex->z = -srcVertex->z;
                dstVertex->w = srcVertex->w;
                dstVertex->u = srcVertex->u;
                dstVertex->v = srcVertex->v;
                dstVertex->r = (srcVertex->r / 255.0);
                dstVertex->g = (srcVertex->g / 255.0);
                dstVertex->b = (srcVertex->b / 255.0);
                dstVertex->a = (srcVertex->a / 255.0);
            }

            numTrianglesInBatch++;
            numTrianglesProcessed++;
        }

        /* If we're at the end of the current batch, render its triangles and
         * start a new batch.*/
        {
            const int isEndOfTriangles = (numTrianglesProcessed >= numTriangles);

            const int hasTexture = (triangle->texture != NULL);
            const uint32_t currentApiId = (hasTexture? triangle->texture->apiId : 0);
            
            const struct kelpo_polygon_triangle_s *nextTriangle = (isEndOfTriangles? NULL : (triangle + 1));
            const int nextHasTexture = (nextTriangle? (nextTriangle->texture != NULL) : 0);
            const uint32_t nextApiId = (nextHasTexture? nextTriangle->texture->apiId : 0);
            const int isEndOfBatch = (isEndOfTriangles || (nextApiId != currentApiId));

            if (isEndOfBatch)
            {
                const unsigned numVerts = (3 * numTrianglesInBatch);

                if (!hasTexture)
                {
                    glDisable(GL_TEXTURE_2D);
                }
                else
                {
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, currentApiId);
                }

                glBufferSubData(GL_ARRAY_BUFFER,
                                0,
                                (GLsizeiptr)(sizeof(struct gl3_vertex_s) * numVerts),
                                vertexCache);

                glDrawArrays(GL_TRIANGLES, 0, numVerts);

                if (isEndOfTriangles)
                {
                    break;
                }

                numTrianglesInBatch = 0;
            }
        }

        triangle++;
    }

    return 1;
}
