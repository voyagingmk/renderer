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
        CreateGBufferEvent(size_t width, size_t height, std::string aliasName):
			width(width),
			height(height),
			aliasName(aliasName)
        {}
		size_t width;
		size_t height;
		std::string aliasName;
    };

	class DestroyGBufferEvent: public ecs::Event<DestroyGBufferEvent>{
	public:
		DestroyGBufferEvent(std::string aliasName) :
			aliasName(aliasName)
		{}
		std::string aliasName;
	};
    
    class UseGBufferEvent : public ecs::Event<UseGBufferEvent> {
    public:
        UseGBufferEvent(std::string aliasName):
			aliasName(aliasName)
        {}
		std::string aliasName;
    };   
    
    class UnuseGBufferEvent : public ecs::Event<UnuseGBufferEvent> {
    public:
        UnuseGBufferEvent(std::string aliasName):
			aliasName(aliasName)
        {}
		std::string aliasName;
    };
    
}

#endif
