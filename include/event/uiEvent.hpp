#ifndef RENDERER_EVENT_UI_HPP
#define RENDERER_EVENT_UI_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {

    class SetupUIEvent: public ecs::Event<SetupUIEvent> {
	public:
		SetupUIEvent() {}
	};

    class DrawUIEvent: public ecs::Event<DrawUIEvent> {
    public:
        DrawUIEvent() {}
    };
    

};


#endif
