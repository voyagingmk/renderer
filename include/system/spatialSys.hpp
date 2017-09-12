#ifndef RENDERER_SYSTEM_SPATIAL_HPP
#define RENDERER_SYSTEM_SPATIAL_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/spatialEvent.hpp"



using namespace ecs;

namespace renderer {
	class SpatialSystem : public System<SpatialSystem>, public Receiver<SpatialSystem>
	{
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;
        void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
        void receive(const UpdateSpatialDataEvent& evt);
	};

};

#endif
