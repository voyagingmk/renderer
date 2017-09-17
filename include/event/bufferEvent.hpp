#ifndef RENDERER_EVENT_BUFFER_HPP
#define RENDERER_EVENT_BUFFER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {

    class DrawMeshBufferEvent : public ecs::Event<DrawMeshBufferEvent> {
    public:
        DrawMeshBufferEvent(ecs::Object obj):
            obj(obj)
        {}
        ecs::Object obj;
    };

    
    class CreateGBufferEvent : public ecs::Event<CreateGBufferEvent> {
    public:
        CreateGBufferEvent(ecs::Object obj):
            obj(obj)
        {}
        ecs::Object obj;
    };
    
    class UseGBufferEvent : public ecs::Event<UseGBufferEvent> {
    public:
        UseGBufferEvent(ecs::Object obj):
            obj(obj)
        {}
        ecs::Object obj;
    };   
    
    class UnuseGBufferEvent : public ecs::Event<UnuseGBufferEvent> {
    public:
        UnuseGBufferEvent(ecs::Object obj):
            obj(obj)
        {}
        ecs::Object obj;
    };
    
}

#endif
