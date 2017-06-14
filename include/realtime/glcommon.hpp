#ifndef RENDERER_GLCOMMON_HPP
#define RENDERER_GLCOMMON_HPP

#include "base.hpp"

    
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


typedef uint32_t bufferID;

struct BufferSet {
    size_t triangles;
    bufferID vao;
    bufferID vbo;
    bufferID ebo;
};

typedef std::map<std::string, BufferSet> BufferDict;


class FrameBuf {
public:
    FboID fboID;
    TexID texID;
    FboID innerFboID;
    TexID innerTexID;
    BufType depthType;
    TexID depthTexID; // include stencil
    RboID depthRboID; // include stencil
    size_t width;
    size_t height;
    size_t MSAA; // default: 0
public:
    FrameBuf():
        fboID(0),
        texID(0),
        innerFboID(0),
        innerTexID(0),
        depthType(BufType::RBO),
        depthTexID(0),
        width(1),
        height(1),
        MSAA(0)
    {}
    TexID getTexID() {
        if (MSAA) {
            return innerTexID;
        }
        return texID;
    }
    void debug() {
        printf("FBO:%d, TexID:%d, depthTexID:%d, width:%d, height:%d \n",
               fboID, texID, depthTexID, (int)width, (int)height);
    }
};

#endif
