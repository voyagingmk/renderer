#ifndef RENDERER_SYSTEM_ENV_HPP
#define RENDERER_SYSTEM_ENV_HPP

#include "base.hpp"
#include "../ecs/ecs.hpp"
#include "../com/sdlContext.hpp"
#include "../com/renderMode.hpp"
#include "../com/keyState.hpp"
#include "../com/screenSize.hpp"
#include "event/winEvent.hpp"



using namespace ecs;

namespace renderer {
	
	class EnvSystem : public System<EnvSystem>, public Receiver<EnvSystem>
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

		void receive(const CustomSDLKeyboardEvent &evt);
	};

}
#endif