#ifndef RENDERER_EVENT_SPATIAL_HPP
#define RENDERER_EVENT_SPATIAL_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/shader.hpp"

namespace renderer {

	class UpdateSpatialDataEvent : public ecs::Event<UpdateSpatialDataEvent> {
	public:
		UpdateSpatialDataEvent(ecs::Object obj, uint32_t flag = 0):
            obj(obj),
            flag(flag)
		{}
        ecs::Object obj;
        uint32_t flag;
	};


}

#endif
