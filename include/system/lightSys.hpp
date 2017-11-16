#ifndef RENDERER_SYSTEM_LIGHT_HPP
#define RENDERER_SYSTEM_LIGHT_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/lightEvent.hpp"
#include "event/spatialEvent.hpp"
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

		void receive(const UpdateSpatialDataEvent &evt);

		void receive(const EnableLightShadowEvent &evt);

		void receive(const DisableLightShadowEvent &evt);

		void updatePointLight(Object obj);
    };     
}

#endif