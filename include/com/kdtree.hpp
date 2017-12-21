#ifndef RENDERER_COM_KDTREE_HPP
#define RENDERER_COM_KDTREE_HPP


#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/bbox.hpp"


using namespace ecs;

namespace renderer {

	struct KdAccelNode {
		void InitInterior(int axis, int ac, float s) {
			split = s;
			flags = axis;
			aboveChild |= (ac << 2);
		}
		float SplitPos() const { return split; }
		int numObjs() const { return nObjs >> 2; }
		int SplitAxis() const { return flags & 3; }
		bool IsLeaf() const { return (flags & 3) == 3; }
		int AboveChild() const { return aboveChild >> 2; }
		union {
			float split;                 // Interior
			int oneObj;            // Leaf
			int objIndicesOffset;  // Leaf
		};

		union {
			int flags;       // Both
			int nObjs;      // Leaf
			int aboveChild;  // Interior
		};
	};

	enum class EdgeType { Start, End };

	struct BoundEdge {
		// BoundEdge Public Methods
		BoundEdge() {}
		BoundEdge(float t, int objNum, bool starting) : t(t), objNum(objNum) {
			type = starting ? EdgeType::Start : EdgeType::End;
		}
		float t;
		int objNum;
		EdgeType type;
	};

	struct KdToDo {
		const KdAccelNode *node;
		float tMin, tMax;
	};

	struct KdTreeAccel {
		int isectCost, traversalCost, maxObjs;
		float emptyBonus;
		std::vector<ecs::ObjectID> objs;
		std::vector<int> objIndices;
		KdAccelNode *nodes;
		int nAllocedNodes, nextFreeNode;
		BBox bounds;
	};
};

#endif
