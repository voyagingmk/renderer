#ifndef RENDERER_EVENT_BUFFER_HPP
#define RENDERER_EVENT_BUFFER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/bufferCom.hpp"
#include "com/mesh.hpp"

namespace renderer {
    
    class CreateDynamicMeshBufferEvent: public ecs::Event<CreateDynamicMeshBufferEvent> {
    public:
        CreateDynamicMeshBufferEvent(std::string meshName) :
            meshName(meshName)
        {}
        std::string meshName;
    };
    
    
    class UpdateDynamicMeshBufferEvent: public ecs::Event<UpdateDynamicMeshBufferEvent> {
    public:
        UpdateDynamicMeshBufferEvent(std::string meshName,
            size_t vbo_size,
            const void* vbo_data,
            size_t ebo_size,
            const void* ebo_data) :
            meshName(meshName),
            vbo_size(vbo_size),
            vbo_data(vbo_data),
            ebo_size(ebo_size),
            ebo_data(ebo_data)
        {}
        std::string meshName;
        size_t vbo_size;
        const void* vbo_data;
        size_t ebo_size;
        const void* ebo_data;
    };
    
    
    class BindDynamicMeshBufferEvent: public ecs::Event<BindDynamicMeshBufferEvent> {
    public:
        BindDynamicMeshBufferEvent(std::string meshName) :
            meshName(meshName)
        {}
        std::string meshName;
    };
    
    
    class UnbindDynamicMeshBufferEvent: public ecs::Event<UnbindDynamicMeshBufferEvent> {
    public:
        UnbindDynamicMeshBufferEvent(std::string meshName) :
            meshName(meshName)
        {}
        std::string meshName;
    };
    
	class CreateMeshBufferEvent: public ecs::Event<CreateMeshBufferEvent> {
	public:
		CreateMeshBufferEvent(MeshID meshID) :
			meshID(meshID),
			meshName("")
		{}
		CreateMeshBufferEvent(std::string meshName) :
			meshID(0),
			meshName(meshName)
		{}
		std::string meshName;
		MeshID meshID;
	};

	class CreateSkyboxBufferEvent : public ecs::Event<CreateSkyboxBufferEvent> {
	public:
		CreateSkyboxBufferEvent(MeshID meshID) :
			meshID(meshID)
		{}
		MeshID meshID;
	};

	class DrawMeshBufferEvent : public ecs::Event<DrawMeshBufferEvent> {
	public:
		DrawMeshBufferEvent(MeshID meshID, SubMeshIdx subMeshIdx) :
			meshID(meshID),
			meshName(""),
			subMeshIdx(subMeshIdx)
		{}
		DrawMeshBufferEvent(std::string meshName, SubMeshIdx subMeshIdx) :
			meshID(0),
			meshName(meshName),
			subMeshIdx(subMeshIdx)
		{}
		std::string meshName;
		MeshID meshID;
		SubMeshIdx subMeshIdx;
	};

	class BindInstanceBufferEvent : public ecs::Event<BindInstanceBufferEvent> {
	public:
		BindInstanceBufferEvent(MeshID meshID, SubMeshIdx subMeshIdx, std::string insBufferName) :
			meshID(meshID),
			subMeshIdx(subMeshIdx),
			insBufferName(insBufferName)
		{}
		MeshID meshID;
		SubMeshIdx subMeshIdx;
		std::string insBufferName;
	};

	class UnbindInstanceBufferEvent : public ecs::Event<UnbindInstanceBufferEvent> {
	public:
		UnbindInstanceBufferEvent(MeshID meshID, SubMeshIdx subMeshIdx) :
			meshID(meshID),
			subMeshIdx(subMeshIdx)
		{}
		MeshID meshID;
		SubMeshIdx subMeshIdx;
	};


    class DrawSubMeshBufferEvent : public ecs::Event<DrawSubMeshBufferEvent> {
    public:
		DrawSubMeshBufferEvent(MeshBufferRef& buf):
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
		CreateInstanceBufferEvent(std::string aliasName):
			aliasName(aliasName)
		{}
		std::string aliasName;
	};

	class DestroyInstanceBufferEvent : public ecs::Event<DestroyInstanceBufferEvent> {
	public:
		DestroyInstanceBufferEvent(std::string aliasName) :
			aliasName(aliasName)
		{}
		std::string aliasName;
	};

	class UpdateInstanceBufferEvent : public ecs::Event<UpdateInstanceBufferEvent> {
	public:
		UpdateInstanceBufferEvent(std::string aliasName,
			size_t instanceNum,
			size_t perBytes,
			void* data) :
			aliasName(aliasName),
			instanceNum(instanceNum),
			perBytes(perBytes),
			data(data)
		{}
		std::string aliasName;
		size_t instanceNum;
		size_t perBytes;
		void* data;
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
