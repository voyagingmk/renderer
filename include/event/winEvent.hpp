#ifndef RENDERER_EVENT_WIN_HPP
#define RENDERER_EVENT_WIN_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {

	class CustomSDLEvent : public ecs::Event<CustomSDLEvent> {
	public:
		CustomSDLEvent(ecs::Object obj, SDL_Event e) :
			obj(obj), e(e) {}
		ecs::Object obj;
		SDL_Event e;
	};

	class CustomSDLKeyboardEvent : public ecs::Event<CustomSDLKeyboardEvent> {
	public:
		CustomSDLKeyboardEvent(ecs::Object obj, SDL_KeyboardEvent e) :
			obj(obj), e(e) {}
		ecs::Object obj;
		SDL_KeyboardEvent e;
	};

	class CustomSDLMouseMotionEvent : public ecs::Event<CustomSDLMouseMotionEvent> {
	public:
		CustomSDLMouseMotionEvent(ecs::Object obj, SDL_MouseMotionEvent e) :
			obj(obj), e(e) {}
		ecs::Object obj;
		SDL_MouseMotionEvent e;
	};

	class CustomSDLMouseButtonEvent : public ecs::Event<CustomSDLMouseButtonEvent> {
	public:
		CustomSDLMouseButtonEvent(ecs::Object obj, SDL_MouseButtonEvent e) :
			obj(obj), e(e) {}
		ecs::Object obj;
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
