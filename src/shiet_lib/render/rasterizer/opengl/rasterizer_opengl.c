#include <gl/gl.h>
#include <gl/glext.h>
#include "shiet_lib/render/rasterizer/opengl/rasterizer_opengl.h"

void shiet_rasterizer_opengl__clear_frame(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return;
}
