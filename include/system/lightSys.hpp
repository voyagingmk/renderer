#ifndef RENDERER_SYSTEM_LIGHT_HPP
#define RENDERER_SYSTEM_LIGHT_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/lightEvent.hpp"
#include "com/lightCom.hpp"

using namespace ecs;

namespace renderer {	
    class LightSystem : public System<LightSystem>, public Receiver<LightSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
		
        void receive(const UpdatePointLightEvent &evt);	
        
        void receive(const AddPointLightEvent &evt);

		void updatePointLight(Object obj);
    };     
}

#endif