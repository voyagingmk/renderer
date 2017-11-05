#ifndef RENDERER_COM_SDLCONTEXT_HPP
#define RENDERER_COM_SDLCONTEXT_HPP

#include "base.hpp"

namespace renderer {
	struct SDLContext {
		SDLContext(size_t w, size_t h) :
			width(w),
			height(h),
			shouldExit(false),
			showCursor(false),
			win(nullptr),
			rendererSDL(nullptr)
		{}
		size_t width;
		size_t height;
		bool shouldExit;
		bool showCursor;
		SDL_Window * win;
		SDL_Renderer * rendererSDL;
		SDL_GLContext glContext;
	};

}

#endif