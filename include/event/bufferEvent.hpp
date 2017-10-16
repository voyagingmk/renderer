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

	class AddColorBufferEvent : public ecs::Event<AddColorBufferEvent> {
	public:
		AddColorBufferEvent(std::string aliasName, ColorBufferRef buf) :
			aliasName(aliasName),
			buf(buf)
		{}
		std::string aliasName;
		ColorBufferRef buf;
    };
    
    class CreateDpethBufferEvent : public ecs::Event<CreateDpethBufferEvent> {
    public:
        CreateDpethBufferEvent(std::string aliasName,
                               std::string texAliasname,
                               DepthTexType dtType,
                               size_t width):
            width(width),
            dtType(dtType),
            aliasName(aliasName),
            texAliasname(texAliasname)
        {}
        size_t width;
        DepthTexType dtType;
        std::string aliasName;
        std::string texAliasname;
    };
    

	class CreateColorBufferEvent : public ecs::Event<CreateColorBufferEvent> {
	public:
		CreateColorBufferEvent(size_t width, size_t height,
            int internalFormat,
            int dataType,
			BufType depthType,
			size_t MSAA,
            int texParam,
			std::string aliasName) :
			width(width),
			height(height),
            internalFormat(internalFormat),
            dataType(dataType),
			depthType(depthType),
			MSAA(MSAA),
            texParam(texParam),
			aliasName(aliasName)
		{}
		size_t width;
		size_t height;
		int internalFormat;
        int dataType;
		BufType depthType;
		size_t MSAA;
        int texParam;
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
