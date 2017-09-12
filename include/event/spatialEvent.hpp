#ifndef RENDERER_EVENT_SPATIAL_HPP
#define RENDERER_EVENT_SPATIAL_HPP

#include "base.hpp"

namespace renderer {

	class UpdateSpatialDataEvent : public ecs::Event<UpdateSpatialDataEvent> {
	public:
		UpdateSpatialDataEvent(ecs::Object obj):
            obj(obj)
		{}
        ecs::Object obj;
	};

}

#endif
