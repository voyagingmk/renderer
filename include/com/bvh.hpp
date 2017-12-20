#ifndef RENDERER_COM_BVH_HPP
#define RENDERER_COM_BVH_HPP


#include "base.hpp"
#include "ecs/ecs.hpp"
#include "mesh.hpp"

namespace renderer {
	struct BVHBuildNode {
		void InitLeaf(int first, int n, const BBox &b) {
			firstPrimOffset = first;
			nObjs = n;
			bounds = b;
			children[0] = children[1] = nullptr;
		}
		void InitInterior(Axis axis, BVHBuildNode *c0, BVHBuildNode *c1) {
			children[0] = c0;
			children[1] = c1;
			bounds = Union(c0->bounds, c1->bounds);
			splitAxis = axis;
			nObjs = 0;
		}
		BBox bounds;
		BVHBuildNode *children[2];
		Axis splitAxis;
		int firstPrimOffset, nObjs;
	};
	struct LBVHTreelet {
		int startIndex, nObjs;
		BVHBuildNode *buildNodes;
	};



	// BVHAccel Local Declarations
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

	// BVHAccel Declarations
	class BVHAccel {
	public:
		// BVHAccel Public Types
		enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };
		BBox WorldBound() const;
		~BVHAccel();

		// BVHAccel Private Data
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
#endif  // PBRT_ACCELERATORS_BVH_H
