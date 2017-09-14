#ifndef RENDERER_COM_MESHES_HPP
#define RENDERER_COM_MESHES_HPP

#include "base.hpp"
#include "vertex.hpp"


namespace renderer {
    class OneMesh {
        public:
            Vertices vertices;
		    UIntArray indexes;
    };

    class Meshes {
        public:
			Meshes() {}
			Meshes(std::vector<OneMesh>& meshes):
				meshes(meshes)
			{}
            std::vector<OneMesh> meshes;
    };
};

#endif
