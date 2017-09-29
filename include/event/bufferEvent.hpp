#ifndef RENDERER_EVENT_BUFFER_HPP
#define RENDERER_EVENT_BUFFER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

namespace renderer {

	
	class CreateMeshBufferEvent: public ecs::Event<CreateMeshBufferEvent> {
	public:
		CreateMeshBufferEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};

	class CreateSkyboxBufferEvent : public ecs::Event<CreateSkyboxBufferEvent> {
	public:
		CreateSkyboxBufferEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};

    class DrawMeshBufferEvent: public ecs::Event<DrawMeshBufferEvent> {
    public:
        DrawMeshBufferEvent(ecs::Object obj):
            obj(obj)
        {}
        ecs::Object obj;
    };

	class CreateColorBufferEvent : public ecs::Event<CreateColorBufferEvent> {
	public:
		CreateColorBufferEvent(size_t width, size_t height,
			BufType depthType,
			size_t MSAA,
			std::string aliasName) :
			width(width),
			height(height),
			depthType(depthType),
			MSAA(MSAA),
			aliasName(aliasName)
		{}
		size_t width;
		size_t height;
		BufType depthType;
		size_t MSAA;
		std::string aliasName;
	};

	class DestroyColorBufferEvent : public ecs::Event<DestroyColorBufferEvent> {
	public:
		DestroyColorBufferEvent(std::string aliasName) :
			aliasName(aliasName)
		{}
		std::string aliasName;
	};

	class UseColorBufferEvent : public ecs::Event<UseColorBufferEvent> {
	public:
		UseColorBufferEvent(std::string aliasName) :
			aliasName(aliasName)
		{}
		std::string aliasName;
	};

	class UnuseColorBufferEvent : public ecs::Event<UnuseColorBufferEvent> {
	public:
		UnuseColorBufferEvent(std::string aliasName) :
			aliasName(aliasName)
		{}
		std::string aliasName;
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

	
	class CopyGBufferDepthEvent : public ecs::Event<CopyGBufferDepthEvent> {
	public:
		CopyGBufferDepthEvent(std::string aliasName) :
			aliasName(aliasName)
		{}
		std::string aliasName;
	};
};

#endif
