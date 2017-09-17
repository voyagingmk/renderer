#ifndef RENDERER_EVENT_WIN_HPP
#define RENDERER_EVENT_WIN_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {

	class SetupSDLEvent: public ecs::Event<SetupSDLEvent> {
	public:
		SetupSDLEvent(size_t winWidth, size_t winHeight):
			winWidth(winWidth),
			winHeight(winHeight)
		{}
		size_t winWidth;
		size_t winHeight;
	};

	class CustomSDLEvent: public ecs::Event<CustomSDLEvent> {
	public:
		CustomSDLEvent(SDL_Event e) :
			e(e) {}
		SDL_Event e;
	};

	class CustomSDLKeyboardEvent : public ecs::Event<CustomSDLKeyboardEvent> {
	public:
		CustomSDLKeyboardEvent(SDL_KeyboardEvent e) :
			e(e) {}
		SDL_KeyboardEvent e;
	};

	class CustomSDLMouseMotionEvent : public ecs::Event<CustomSDLMouseMotionEvent> {
	public:
		CustomSDLMouseMotionEvent(SDL_MouseMotionEvent e) :
			e(e) {}
		SDL_MouseMotionEvent e;
	};

	class CustomSDLMouseButtonEvent : public ecs::Event<CustomSDLMouseButtonEvent> {
	public:
		CustomSDLMouseButtonEvent(SDL_MouseButtonEvent e) :
			e(e) {}
		SDL_MouseButtonEvent e;
	};


	class WinResizeEvent : public ecs::Event<WinResizeEvent> {
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
