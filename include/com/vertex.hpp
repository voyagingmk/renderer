#ifndef RENDERER_COM_VERTEX_HPP
#define RENDERER_COM_VERTEX_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
    
    struct Vertex {
        // Position
        Vector3dF position;
        // Normal
        Normal3dF normal;
        Vector3dF color;
        // TexCoords
        Vector2dF texCoords;
    };
    
	typedef std::vector<Vertex> Vertices;
};


#endif
