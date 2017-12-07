#ifndef RENDERER_COM_MESHES_HPP
#define RENDERER_COM_MESHES_HPP

#include "base.hpp"
#include "vertex.hpp"


namespace renderer {
    class OneMesh {
        public:
			OneMesh():
				matIdx(0)
			{}
            Vertices vertices;
		    UIntArray indexes;
			unsigned int matIdx;
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

	struct MeshesRef {
		MeshesRef(ecs::ObjectID id):
			objID(id)
		{}
		ecs::ObjectID objID;

	};
};

#endif
