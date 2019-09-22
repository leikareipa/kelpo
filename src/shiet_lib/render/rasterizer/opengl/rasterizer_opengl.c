#include <stddef.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include "shiet_lib/render/rasterizer/opengl/rasterizer_opengl.h"
#include "shiet/polygon/triangle/triangle.h"
#include "shiet/polygon/texture.h"

void shiet_rasterizer_opengl__clear_frame(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return;
}

void shiet_rasterizer_opengl__draw_triangles(struct shiet_polygon_triangle_s *const triangles,
                                             const unsigned numTriangles)
{
	unsigned i;

	for (i = 0; i < numTriangles; i++)
	{
		const struct shiet_polygon_texture_s *const texture = triangles[i].material.texture;

		/* If the triangle has no texture, draw it with a solid fill of its base color.*/
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
		/* Otherwise, draw the triangle with a texture.*/
		else
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture->apiId);
			glColor4ub(255, 255, 255, 255);

			glBegin(GL_TRIANGLES);
				glTexCoord2f(triangles[i].vertex[0].u, triangles[i].vertex[0].v);
				glVertex2f(triangles[i].vertex[0].x, -triangles[i].vertex[0].y);

				glTexCoord2f(triangles[i].vertex[1].u, triangles[i].vertex[1].v);
				glVertex2f(triangles[i].vertex[1].x, -triangles[i].vertex[1].y);

				glTexCoord2f(triangles[i].vertex[2].u, triangles[i].vertex[2].v);
				glVertex2f(triangles[i].vertex[2].x, -triangles[i].vertex[2].y);
			glEnd();
		}
	}

	return;
}
