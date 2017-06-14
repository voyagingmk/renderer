#ifndef RENDERER_GLUTILS_HPP
#define RENDERER_GLUTILS_HPP

#include "base.hpp"

#ifdef USE_GL
static bool checkGLError(const char* file, int line) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
        const GLubyte* errString = gluErrorString(err);
		printf("%s\n",errString);
        printf("errcode:%d\n", (int)err);
        printf("This fake error is in %s on line %d\n ", file, line);
		return false;
	}
	return true;
}

#define CheckGLError checkGLError(__FILE__, __LINE__);

#endif

#endif
