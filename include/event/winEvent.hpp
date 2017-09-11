#ifndef RENDERER_EVENT_WIN_HPP
#define RENDERER_EVENT_WIN_HPP

#include "base.hpp"

namespace renderer {

	class WinResizeEvent {
	public:
		WinResizeEvent(size_t w, size_t h):
			width(w),
			height(h)
		{}
		size_t width;
		size_t height;
	};

}

#endif
