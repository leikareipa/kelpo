#include <stddef.h>
#include "shiet_lib/renderer/rasterizer/opengl/rasterizer_opengl.h"
#include "shiet/polygon/triangle/triangle.h"
#include "shiet/polygon/texture.h"

#include <gl/gl.h>
#include <gl/glext.h>

void shiet_rasterizer_opengl__clear_frame(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    return;
}

void shiet_rasterizer_opengl__upload_texture(struct shiet_polygon_texture_s *const texture)
{
    glGenTextures(1, (GLuint*)&texture->apiId);
    glBindTexture(GL_TEXTURE_2D, texture->apiId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (texture->filtering == SHIET_TEXTURE_FILTER_LINEAR? GL_LINEAR : GL_NEAREST));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (texture->filtering == SHIET_TEXTURE_FILTER_LINEAR? GL_LINEAR : GL_NEAREST));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixelArray);

    return;
}

void shiet_rasterizer_opengl__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                             const unsigned numTriangles)
{
    unsigned i;

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
            const float w = triangles[i].vertex[0].w;
            const float wInv = (1 / w);

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture->apiId);
            glColor4ub(255, 255, 255, 255);

            glBegin(GL_TRIANGLES);
                glTexCoord4f((triangles[i].vertex[0].u / w), (triangles[i].vertex[0].v / w), 0, wInv);
                glVertex2f(triangles[i].vertex[0].x, -triangles[i].vertex[0].y);

                glTexCoord4f((triangles[i].vertex[1].u / w), (triangles[i].vertex[1].v / w), 0, wInv);
                glVertex2f(triangles[i].vertex[1].x, -triangles[i].vertex[1].y);

                glTexCoord4f((triangles[i].vertex[2].u / w), (triangles[i].vertex[2].v / w), 0, wInv);
                glVertex2f(triangles[i].vertex[2].x, -triangles[i].vertex[2].y);
            glEnd();
        }
    }

    return;
}
