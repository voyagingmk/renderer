#ifndef RENDERER_COM_GLCOMMON_HPP
#define RENDERER_COM_GLCOMMON_HPP

#include "base.hpp"
#include "../com/geometry.hpp"

namespace renderer {

    
#ifdef USE_GL
typedef GLuint TexID;
typedef GLuint FboID;
typedef GLuint RboID;
#else
typedef uint32_t TexID;
typedef uint32_t FboID;
typedef uint32_t RboID;
#endif

enum class BufType {
    Tex = 1, // texture
    RBO = 2  // render buffer
};
enum class TexType {
    Tex2D = 1,
    CubeMap = 2
};

enum class DepthTexType {
    DepthOnly = 1,
    DepthStencil = 2,
    CubeMap = 3
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

class FrameBuf {
public:
    FboID fboID;
    TexRef tex;
    FboID innerFboID;
    TexRef innerTex;
    BufType depthType;
    TexRef depthTex; // include stencil
    RboID depthRboID; // include stencil
    size_t width;
    size_t height;
    size_t MSAA; // default: 0
public:
    FrameBuf():
        fboID(0),
        innerFboID(0),
        depthType(BufType::RBO),
        width(1),
        height(1),
        MSAA(0)
    {}
    TexRef getTexRef() {
        if (MSAA) {
            return innerTex;
        }
        return tex;
    }
    void debug() {
        printf("FBO:%d, TexID:%d, depthTexID:%d, width:%d, height:%d \n",
               fboID, tex.texID, depthTex.texID, (int)width, (int)height);
    }
};

struct Character {
    TexRef      texRef;  // ID handle of the glyph texture
    Point2dI    Size;       // Size of glyph
    Point2dI    Bearing;    // Offset from baseline to left/top of glyph
    int         Advance;    // Offset to advance to next glyph
};
    

typedef std::map<std::string, TexRef> TextureDict;
};

#endif
