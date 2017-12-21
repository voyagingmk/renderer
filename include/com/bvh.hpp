#ifndef RENDERER_COM_BVH_HPP
#define RENDERER_COM_BVH_HPP


#include "base.hpp"
#include "ecs/ecs.hpp"
#include "mesh.hpp"

namespace renderer {
    
	struct BVHBuildNode {
		BBox bounds;
		BVHBuildNode *children[2];
		Axis splitAxis;
		int firstObjOffset, nObjs;
	};
    
	struct LBVHTreelet {
		int startIndex, nObjs;
		BVHBuildNode *buildNodes;
	};

	struct BVHObjInfo {
		BVHObjInfo() {}
		BVHObjInfo(size_t objNumber, const BBox &bounds)
			: objNumber(objNumber),
			bounds(bounds),
			centroid(.5f * bounds.pMin + .5f * bounds.pMax) {}
		size_t objNumber;
		BBox bounds;
		Vector3dF centroid;
	};

	struct LinearBVHNode {
		BBox bounds;
		union {
			uint32_t objsOffset;   // leaf
			uint32_t secondChildOffset;  // interior
		};
		uint16_t nObjs;  // 0 -> interior node
		Axis axis;          // interior node: xyz
		uint8_t pad[1];        // ensure 32 byte total size
	};

	class BVHAccel {
	public:
		enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };
		int maxObjsInNode;
		SplitMethod splitMethod;
		std::vector<ecs::ObjectID> objs;
		LinearBVHNode *nodes = nullptr;
	};

	struct MortonObj {
		int objIndex;
		uint32_t mortonCode;
	};

};
#endif
