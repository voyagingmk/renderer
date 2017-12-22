#ifndef RENDERER_COM_BVH_HPP
#define RENDERER_COM_BVH_HPP


#include "base.hpp"
#include "ecs/ecs.hpp"
#include "mesh.hpp"

namespace renderer {
    
	struct BVHBuildNode {
		BBox bounds;
        BVHBuildNode *children[2] = {nullptr, nullptr};
		Axis splitAxis;
		int firstObjOffset = 0, nObjs = 0;
	};
    
	struct LBVHTreelet {
        LBVHTreelet(int startIndex, int nObjs, BVHBuildNode *buildNodes):
            startIndex(startIndex),
            nObjs(nObjs),
            buildNodes(buildNodes)
        {}
		int startIndex = 0, nObjs = 0;
		BVHBuildNode *buildNodes = nullptr;
	};

	struct BVHObjInfo {
		BVHObjInfo() {}
		BVHObjInfo(size_t objNumber, const BBox &bounds)
			: objNumber(objNumber),
			bounds(bounds),
			centroid(.5f * bounds.pMin + .5f * bounds.pMax) {}
		size_t objNumber = 0;
		BBox bounds;
		Vector3dF centroid;
	};

	struct LinearBVHNode {
		BBox bounds;
		union {
			uint32_t objsOffset = 0;   // leaf
			uint32_t secondChildOffset;  // interior
		};
		uint16_t nObjs = 0;  // 0 -> interior node
		Axis axis;          // interior node: xyz
		uint8_t pad[1];        // ensure 32 byte total size
	};

	class BVHAccel {
	public:
		enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };
		int maxObjsInNode = 0;
        SplitMethod splitMethod;
		std::vector<ecs::ObjectID> objs;
		LinearBVHNode *nodes = nullptr;
	};

	struct MortonObj {
		int objIndex = 0;
		uint32_t mortonCode = 0;
	};

};
#endif
