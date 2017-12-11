#ifndef RENDERER_COM_BUFFER_HPP
#define RENDERER_COM_BUFFER_HPP

#include "base.hpp"
#include "mesh.hpp"
#include "materialCom.hpp"

namespace renderer {
    
    #ifdef USE_GL
    typedef GLuint TexID; // texture buffer
    typedef GLuint FboID; // frame buffer object 
    typedef GLuint RboID; // render buffer object
	typedef GLuint InstanceBufID; // instance buffer object
    #else
    typedef uint32_t TexID;
    typedef uint32_t FboID;
    typedef uint32_t RboID;
	typedef uint32_t InstanceBufID;
    #endif

    enum class TexType {
        Tex2D = 1,
        CubeMap = 2
    };

    enum class DepthTexType {
        DepthOnly = 1,
        DepthStencil = 2,
        CubeMap = 3
    };


    enum class BufType {
        None = 0, // no buf
        Tex = 1, // texture
        RBO = 2  // render buffer
    };

    class TexRef {
    public:
        TexID texID;
        TexType type;
        size_t width;
        size_t height;
    public:
        TexRef():
        texID(0),
        type(TexType::Tex2D),
        width(1),
        height(1)
        {}
    };

	typedef uint32_t BufferID;

	class FrameBufferBase {
	public:
		FrameBufferBase() :
			fboID(0),
			width(0),
			height(0)
		{}
		FboID fboID;
		size_t width;
		size_t height;
		TexRef depthTex;
	};


	class ColorBufferRef : public FrameBufferBase {
	public:
		TexRef tex;
		size_t MSAA; // default: 0
		FboID innerFboID;
		TexRef innerTex;
		BufType depthType;
		RboID depthRboID; // include stencil
	};

	class GBufferRef : public FrameBufferBase {
	public:
		RboID depthRboID; // include stencil
		TexID posTexID;
		TexID normalTexID; 
		TexID albedoTexID;
		TexID pbrTexID;
	};

	struct InstanceBufferRef {
		InstanceBufferRef():
			bufID(0),
			instanceNum(1)
		{}
		InstanceBufID bufID;
		uint32_t instanceNum;
	};

	struct MeshBufferRef {
		MeshBufferRef():
			triangles(0),
			vao(0),
			vbo(0),
			ebo(0),
			vboIns(0), 
			instanced(false),
			noIndices(false)
		{}
		size_t triangles;
		BufferID vao;
		BufferID vbo;
		BufferID ebo;
		BufferID vboIns;
		InstanceBufferRef insBuf;
		bool instanced;
		bool noIndices;
	};


	typedef std::vector<MeshBufferRef> MeshBufferRefs;
	typedef std::map<MeshID, MeshBufferRefs> MeshBuffersDict;


	struct MeshBuffersSet {
		MeshBuffersDict buffersDict;
	};

	typedef std::map<std::string, GBufferRef> GBufferDict;
	typedef std::map<std::string, ColorBufferRef> ColorBufferDict;
	typedef std::map<std::string, InstanceBufferRef> InstanceBufferDict;

	typedef std::vector<GBufferRef> GBuffers;

	struct GBufferDictCom {
		GBufferDict dict;
	};

	struct ColorBufferDictCom {
		ColorBufferDict dict;
	};

	struct InstanceBufferDictCom {
		InstanceBufferDict dict;
	};
}

#endif
