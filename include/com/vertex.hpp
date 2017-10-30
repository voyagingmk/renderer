#ifndef RENDERER_COM_VERTEX_HPP
#define RENDERER_COM_VERTEX_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
    
    class Vertex {
    public:
        Vertex(Vector3dF p = Vector3dF(0, 0, 0),
				Normal3dF n = Normal3dF(0, 0, 0),
				Vector2dF t = Vector2dF(0, 0),
				Vector3dF ta = Vector3dF(0, 0, 0),
				Vector3dF bta = Vector3dF(0, 0, 0)):
            position(p),
            normal(n),
            texCoords(t),
			tangent(ta),
			bitangent(bta)
        {}
        
        Vector3dF position;
        Normal3dF normal;
        Vector2dF texCoords;
		Vector3dF tangent;
		Vector3dF bitangent;
		// Vector3dF color;
    };
    
	typedef std::vector<Vertex> Vertices;
};


#endif
