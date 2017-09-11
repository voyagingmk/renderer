#ifndef RENDERER_COM_SPATIALDATA_HPP
#define RENDERER_COM_SPATIALDATA_HPP

#include "base.hpp"
#include "geometry.hpp"
#include "quaternion.hpp"

namespace renderer {
    
	struct SpatialData {
		SpatialData(Vector3dF p, Vector3dF s, QuaternionF o):
			pos(p),
            scale(s),
            orientation(o)
		{}
        Vector3dF pos;
        Vector3dF scale;
        QuaternionF orientation;
	};

}

#endif
