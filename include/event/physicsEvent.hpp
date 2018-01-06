#ifndef RENDERER_EVENT_PHYSICS_HPP
#define RENDERER_EVENT_PHYSICS_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "reactphysics3d/reactphysics3d.h"

using namespace reactphysics3d;

namespace renderer {
    
    class CreateCollisionShapeEvent : public ecs::Event<CreateCollisionShapeEvent> {
    public:
        CreateCollisionShapeEvent(ecs::Object obj, decimal bounciness, decimal friction):
            obj(obj),
            bounciness(bounciness),
            friction(friction)
        {}
        ecs::Object obj;
        decimal bounciness;
        decimal friction;
    };
    
    
}

#endif

