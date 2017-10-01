#ifndef RENDERER_COM_VERTEX_HPP
#define RENDERER_COM_VERTEX_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
    
    class Vertex {
    public:
        Vertex(Vector3dF p = Vector3dF(0, 0, 0),
               Normal3dF n = Normal3dF(0, 0, 1),
               Vector3dF c = Vector3dF(1, 1, 1),
               Vector2dF t = Vector2dF(0, 0)):
            position(p),
            normal(n),
            color(c),
            texCoords(t)
        {}
        
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
