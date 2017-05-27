#ifndef RENDERER_GLUTILS_HPP
#define RENDERER_GLUTILS_HPP

#include "base.hpp"

#ifdef USE_GL
bool checkGLError() {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		const GLubyte* errString = gluErrorString(err);
		std::cout << errString << std::endl;
		return false;
	}
	return true;
}
#endif

#endif
