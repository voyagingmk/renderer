#ifndef RENDERER_SYSTEM_RENDERER_HPP
#define RENDERER_SYSTEM_RENDERER_HPP

#include "base.hpp"
#include "../ecs/ecs.hpp"
#include "../com/sdlContext.hpp"
#include "../com/renderMode.hpp"
#include "../com/keyState.hpp"
#include "../com/screenSize.hpp"



using namespace ecs;

namespace renderer {
	class CustomSDLEvent {
	public:
		CustomSDLEvent(Object obj, SDL_Event e) :
			obj(obj), e(e) {}
		Object obj;
		SDL_Event e;
	};

	class CustomSDLKeyboardEvent {
	public:
		CustomSDLKeyboardEvent(Object obj, SDL_KeyboardEvent e) :
			obj(obj), e(e) {}
		Object obj;
		SDL_KeyboardEvent e;
	};

	class CustomSDLMouseMotionEvent {
	public:
		CustomSDLMouseMotionEvent(Object obj, SDL_MouseMotionEvent e) :
			obj(obj), e(e) {}
		Object obj;
		SDL_MouseMotionEvent e;
	};

	class CustomSDLMouseButtonEvent {
	public:
		CustomSDLMouseButtonEvent(Object obj, SDL_MouseButtonEvent e) :
			obj(obj), e(e) {}
		Object obj;
		SDL_MouseButtonEvent e;
	};

	class RendererSystem : public System<RendererSystem>, public Receiver<RendererSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void shutdown(const char *msg);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const ComponentAddedEvent<SDLContext> &evt);

		void receive(const ComponentRemovedEvent<SDLContext> &evt);

		void receive(const CustomSDLMouseMotionEvent &evt)
		{
			std::cout << "CustomSDLMouseMotionEvent: " << std::endl;
		}

		void receive(const CustomSDLMouseButtonEvent &evt)
		{
			std::cout << "CustomSDLMouseButtonEvent: " << std::endl;
		}

		void receive(const CustomSDLEvent &evt)
		{
			auto com = evt.obj.component<SDLContext>();
			switch (evt.e.type)
			{
			case SDL_QUIT:
				com->shouldExit = true;
				break;
			default:
				break;
			}
		}

		void receive(const CustomSDLKeyboardEvent &evt)
		{
			auto context = evt.obj.component<SDLContext>();
			auto renderMode = evt.obj.component<RenderMode>();
			auto keyState = evt.obj.component<KeyState>();

			auto k = evt.e.keysym.sym;
			keyState->state[k] = evt.e.state;

			if (evt.e.state == SDL_PRESSED)
			{
				switch (k)
				{
				case SDLK_ESCAPE:
				{
					context->shouldExit = true;
					break;
				}
				case SDLK_1:
				{
					// normal
					renderMode->mode = RenderModeEnum::Normal;
					break;
				}
				case SDLK_2:
				{
					// depth map
					renderMode->mode = RenderModeEnum::DepthMap;
					break;
				}
				default:
					break;
				}
			}
		}
	};

}
#endif