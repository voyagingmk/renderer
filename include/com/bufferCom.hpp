#ifndef RENDERER_COM_BUFFER_HPP
#define RENDERER_COM_BUFFER_HPP

#include "base.hpp"

namespace renderer {
    
    #ifdef USE_GL
    typedef GLuint TexID; // texture buffer
    typedef GLuint FboID; // frame buffer object 
    typedef GLuint RboID; // render buffer object
    #else
    typedef uint32_t TexID;
    typedef uint32_t FboID;
    typedef uint32_t RboID;
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
		FboID fboID;
		size_t width;
		size_t height;
	};


	class ColorBufferRef : public FrameBufferBase {
	public:
		TexRef tex;
		size_t MSAA; // default: 0
		FboID innerFboID;
		TexRef innerTex;
		BufType depthType;
		TexRef depthTex; // include stencil
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


	struct MeshBufferRef {
	public:
		MeshBufferRef():
			triangles(0),
			vao(0),
			vbo(0),
			ebo(0),
			noIndices(false)
		{}
		size_t triangles;
		BufferID vao;
		BufferID vbo;
		BufferID ebo;
		bool noIndices;

	};


	typedef std::map<std::string, MeshBufferRef> MeshBufferDict;

	typedef std::vector<MeshBufferRef> MeshBufferRefs;

	struct MeshBuffersCom {
		MeshBufferRefs buffers;
	};


	typedef std::map<std::string, GBufferRef> GBufferDict;

	typedef std::vector<GBufferRef> GBuffers;

	struct GBufferDictCom {
		GBufferDict dict;
	};


	typedef std::map<std::string, ColorBufferRef> ColorBufferDict;

	struct ColorBufferDictCom {
		ColorBufferDict dict;
	};
}

#endif
