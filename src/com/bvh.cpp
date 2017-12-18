#include "stdafx.h"
#include "utils/defines.hpp"
#include "com/bvh.hpp"
#include <algorithm>

namespace renderer {

	BBox BVHAccel::WorldBound() const {
		return nodes ? nodes[0].bounds : BBox();
	}

	BVHAccel::~BVHAccel() { FreeAligned(nodes); }



};