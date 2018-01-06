#ifndef RENDERER_EVENT_PHYSICS_HPP
#define RENDERER_EVENT_PHYSICS_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/shader.hpp"

namespace renderer {
    
    class CreateCollisionShapeEvent : public ecs::Event<CreateCollisionShapeEvent> {
    public:
        CreateCollisionShapeEvent(ecs::Object obj):
        obj(obj)
        {}
        ecs::Object obj;
    };
    
    
}

#endif

