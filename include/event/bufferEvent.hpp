#ifndef RENDERER_EVENT_BUFFER_HPP
#define RENDERER_EVENT_BUFFER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {

    class DrawBufferEvent : public ecs::Event<DrawBufferEvent> {
    public:
        DrawBufferEvent(ecs::Object obj):
            obj(obj)
        {}
        ecs::Object obj;
    };
}

#endif
