#ifndef RENDERER_GLUTILS_HPP
#define RENDERER_GLUTILS_HPP

#include "base.hpp"
#include "defines.hpp"
#include "com/color.hpp"

namespace renderer {

#ifdef USE_GL
	static bool CheckGLError(const char* file, int line) {
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			const GLubyte* errString = gluErrorString(err);
			printf("%s\n", errString);
			printf("errcode:%d\n", (int)err);
			printf("This fake error is in %s on line %d\n ", file, line);
			return false;
		}
		return true;
	}

#define CheckGLError CheckGLError(__FILE__, __LINE__);

	static void setViewport(const Viewport& viewport) {
		glViewport(std::get<0>(viewport),
			std::get<1>(viewport),
			std::get<2>(viewport),
			std::get<3>(viewport));
	}


	static void clearView(const Color clearColor, const uint32_t clearBits) {
		glClearColor(clearColor.r(), clearColor.g(), clearColor.b(), clearColor.a());
		glClear(clearBits);
	}
};

#endif

#endif
