#ifndef RENDERER_EVENT_BUFFER_HPP
#define RENDERER_EVENT_BUFFER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/bufferCom.hpp"

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

	class DrawMeshBufferEvent : public ecs::Event<DrawMeshBufferEvent> {
	public:
		DrawMeshBufferEvent(ecs::Object obj) :
			obj(obj)
		{}
		ecs::Object obj;
	};

	class EnabledMeshBufferInstanceEvent : public ecs::Event<EnabledMeshBufferInstanceEvent> {
	public:
		EnabledMeshBufferInstanceEvent(ecs::Object obj, InstanceBufferRef insBuf) :
			obj(obj),
			insBuf(insBuf)
		{}
		ecs::Object obj;
		InstanceBufferRef insBuf;
	};
	


    class DrawOneMeshBufferEvent : public ecs::Event<DrawOneMeshBufferEvent> {
    public:
		DrawOneMeshBufferEvent(MeshBufferRef& buf):
			buf(buf)
        {}
		MeshBufferRef buf;
    };

	class AddColorBufferEvent : public ecs::Event<AddColorBufferEvent> {
	public:
		AddColorBufferEvent(const char * aliasName, ColorBufferRef buf) :
			aliasName(aliasName),
			buf(buf)
		{}
		const char * aliasName;
		ColorBufferRef buf;
    };
    
    class CreateDpethBufferEvent : public ecs::Event<CreateDpethBufferEvent> {
    public:
        CreateDpethBufferEvent(const char* aliasName,
							   const char* texAliasname,
                               DepthTexType dtType,
                               size_t width):
            width(width),
            dtType(dtType),
            aliasName(aliasName),
            texAliasname(texAliasname)
        {}
        size_t width;
        DepthTexType dtType;
		const char* aliasName;
		const char* texAliasname;
    };
    

	class CreateColorBufferEvent : public ecs::Event<CreateColorBufferEvent> {
	public:
		CreateColorBufferEvent(size_t width, size_t height,
            int internalFormat,
            int format,
            int dataType,
			BufType depthType,
			size_t MSAA,
            int texParam,
			const char* aliasName) :
			width(width),
			height(height),
            internalFormat(internalFormat),
            format(format),
            dataType(dataType),
			depthType(depthType),
			MSAA(MSAA),
            texParam(texParam),
			aliasName(aliasName)
		{}
		size_t width;
		size_t height;
		int internalFormat;
        int format;
        int dataType;
		BufType depthType;
		size_t MSAA;
        int texParam;
		const char* aliasName;
	};

	class DestroyColorBufferEvent : public ecs::Event<DestroyColorBufferEvent> {
	public:
		DestroyColorBufferEvent(const char * aliasName) :
			aliasName(aliasName)
		{}
		const char * aliasName;
	};

	class UseColorBufferEvent : public ecs::Event<UseColorBufferEvent> {
	public:
		UseColorBufferEvent(const char * aliasName) :
			aliasName(aliasName)
		{}
		UseColorBufferEvent(const std::string& aliasName) :
		aliasName(aliasName.c_str())
		{}
		const char * aliasName;
	};

	class UnuseColorBufferEvent : public ecs::Event<UnuseColorBufferEvent> {
	public:
		UnuseColorBufferEvent(const char * aliasName) :
			aliasName(aliasName)
		{}
		UnuseColorBufferEvent(const std::string& aliasName) :
			aliasName(aliasName.c_str())
		{}
		const char * aliasName;
	};

	class CreateInstanceBufferEvent : public ecs::Event<CreateInstanceBufferEvent> {
	public:
		CreateInstanceBufferEvent(size_t instanceNum, 
			size_t perBytes,
			void* data,
			const char* aliasName):
			instanceNum(instanceNum),
			perBytes(perBytes),
			data(data),
			aliasName(aliasName)
		{}
		size_t instanceNum;
		size_t perBytes;
		void* data;
		const char* aliasName;
	};

    class CreateGBufferEvent : public ecs::Event<CreateGBufferEvent> {
    public:
        CreateGBufferEvent(size_t width, size_t height, const char* aliasName):
			width(width),
			height(height),
			aliasName(aliasName)
        {}
		size_t width;
		size_t height;
		const char* aliasName;
    };

	class DestroyGBufferEvent: public ecs::Event<DestroyGBufferEvent>{
	public:
		DestroyGBufferEvent(const char * aliasName) :
			aliasName(aliasName)
		{}
		const char * aliasName;
	};
    
    class UseGBufferEvent : public ecs::Event<UseGBufferEvent> {
    public:
        UseGBufferEvent(const char * aliasName):
			aliasName(aliasName)
        {}
		const char * aliasName;
    };   
    
    class UnuseGBufferEvent : public ecs::Event<UnuseGBufferEvent> {
    public:
        UnuseGBufferEvent(const char * aliasName):
			aliasName(aliasName)
        {}
		const char * aliasName;
    };

	
	class CopyGBufferDepth2ColorBufferEvent : public ecs::Event<CopyGBufferDepth2ColorBufferEvent> {
	public:
		CopyGBufferDepth2ColorBufferEvent(const char * aliasName, const char * aliasName2) :
			aliasName(aliasName),
			aliasName2(aliasName2)
		{}
		const char * aliasName;
		const char * aliasName2;
	};
};

#endif
