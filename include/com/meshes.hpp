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
            ~Meshes() {
                meshes.clear();
            }
			Meshes(std::vector<OneMesh>& m)
            {
                std::copy(m.begin(), m.end(),
                   std::back_inserter(meshes));
            }
            std::vector<OneMesh> meshes;
    };
};

#endif
