#ifndef RENDERER_SYSTEM_MATERIAL_HPP
#define RENDERER_SYSTEM_MATERIAL_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/materialEvent.hpp"


using namespace ecs;

namespace renderer {

	class MaterialSystem : public System<MaterialSystem>, public Receiver<MaterialSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const LoadMaterialEvent &evt);
	};
};


#endif