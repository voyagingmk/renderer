#ifndef RENDERER_BVH_HPP
#define RENDERER_BVH_HPP

#include "union.hpp"


namespace renderer {

	class BVHNode {
	public:
		BVHNode():
			left(nullptr),
			right(nullptr)
		{}
		void InitAsLeaf(Shape* s, const BBox &b) {
			shapes.push_back(s);
			bound = b;
			left = right = nullptr;
		}
		void InitAsLeaf(Shapes s, const BBox &b) {
			shapes.insert(shapes.end(), s.begin(), s.end());
			bound = b;
			left = right = nullptr;
		}
		void InitAsInterior(Axis axis, BVHNode* l, BVHNode* r) {
			left = l;
			right = r;
			bound = Union(l->bound, r->bound);
			splitAxis = axis;
		}
		Shapes shapes;
		BBox bound;
		BVHNode* left;
		BVHNode* right;
		Axis splitAxis;
	};

	class BVHShapeInfo {
	public:
		BVHShapeInfo():
			idx(0)
		{}
		BVHShapeInfo(size_t i, const BBox& wb)
			: idx(i),
			bound(wb),
			centroid(.5f * bound.pMin + .5f * bound.pMax) {}
		size_t idx;
		BBox bound;
		Point3dF centroid;
	};

	typedef std::vector<BVHShapeInfo> BVHShapeInfos;

	class BVHTree: public ShapeUnion {
	public:
		BVHTree(Shapes shapes):
			ShapeUnion(shapes)
		{}
		virtual void Init() override;
		virtual int Intersect(Ray&, IntersectResult*) override;
	private:
		BVHNode* recursiveBuild(BVHShapeInfos& shapeInfos, int start, int end);
	private:
		BVHNode* root;
	};
}


#endif // RENDERER_BVH_HPP
