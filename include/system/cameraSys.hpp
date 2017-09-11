#ifndef RENDERER_SYSTEM_CAMERA_HPP
#define RENDERER_SYSTEM_CAMERA_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/winEvent.hpp"


using namespace ecs;

namespace renderer {

	class CameraSystem : public System<CameraSystem>, public Receiver<CameraSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const WinResizeEvent &evt);
	};
};


#endif
