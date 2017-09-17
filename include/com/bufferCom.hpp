#ifndef RENDERER_COM_BUFFER_HPP
#define RENDERER_COM_BUFFER_HPP

#include "base.hpp"

typedef uint32_t BufferID;

struct MeshBufferRef {
    size_t triangles;
    BufferID vao;
    BufferID vbo;
    BufferID ebo;
};

typedef std::map<std::string, MeshBufferRef> MeshBufferDict;

typedef std::vector<MeshBufferRef> MeshBufferDicts;

struct MeshBufferDictsCom {
	MeshBufferDicts dicts;
};


#endif