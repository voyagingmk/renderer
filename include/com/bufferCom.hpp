#ifndef RENDERER_COM_BUFFER_HPP
#define RENDERER_COM_BUFFER_HPP

#include "base.hpp"

typedef uint32_t BufferID;

struct BufferSet {
    size_t triangles;
    BufferID vao;
    BufferID vbo;
    BufferID ebo;
};

typedef std::map<std::string, BufferSet> BufferDict;

#endif