#ifndef RENDERER_COM_SPATIALDATA_HPP
#define RENDERER_COM_SPATIALDATA_HPP

#include "base.hpp"
#include "ecs/setting.hpp"
#include "geometry.hpp"
#include "quaternion.hpp"
#include "transform.hpp"

namespace renderer {
    
	struct SpatialData {
		SpatialData(Vector3dF p, Vector3dF s, QuaternionF o):
			pos(p),
            scale(s),
            orientation(o),
            o2w(Matrix4x4::newIdentity())
		{}
        Vector3dF pos;
        Vector3dF scale;
        QuaternionF orientation;
        Transform4x4 o2w;
	};

	struct SceneGraphNode {
		ecs::ObjectID parent;
		std::vector<ecs::ObjectID> children;
	};

}

#endif
