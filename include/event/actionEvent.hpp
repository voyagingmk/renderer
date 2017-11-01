#ifndef RENDERER_EVENT_ACTION_HPP
#define RENDERER_EVENT_ACTION_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/motionCom.hpp"

namespace renderer {
    
	class AddActionEvent: public ecs::Event<AddActionEvent> {
	public:
        AddActionEvent(ecs::Object obj, const std::string& name, ActionData& actionData) :
            obj(obj),
            name(name),
            actionData(actionData)
		{}
        ecs::Object obj;
        const std::string& name;
        ActionData actionData;
	};


};

#endif
