#include "stdafx.h"
#include "bvh.hpp"
#include "defines.hpp"
#include "intersect_result.hpp"

namespace renderer {
	struct BVHBucket {
		int count = 0;
		BBox bound;
	};


	void BVHTree::Init() {
		if (geometries.size() == 0) {
			return;
		}
		BVHShapeInfos shapeInfos(geometries.size());
		for (size_t i = 0; i < shapeInfos.size(); ++i) {
			shapeInfos[i] = { i, geometries[i]->WorldBound() };
		}
		root = recursiveBuild(shapeInfos, 0, geometries.size());
	}

	BVHNode* BVHTree::recursiveBuild(BVHShapeInfos& shapeInfos, int start, int end) {
		auto pool = GetPool<BVHNode>();
		BVHNode* node = pool->newElement();
		BBox boundAll;
		for (int i = start; i < end; ++i) {
			boundAll = Union(boundAll, shapeInfos[i].bound);
		}
		int count = end - start;
		if (count == 1) {
			size_t idx = shapeInfos[start].idx;
			Shape* shape = geometries[idx];
			node->InitAsLeaf(shape, boundAll);
			return node;
		}
		BBox centroidBound;
		for (int i = start; i < end; ++i) {
			centroidBound = centroidBound.Union(shapeInfos[i].centroid);
		}
		Axis axis = centroidBound.MaximumExtent();
		int mid = (start + end) / 2;
		if (centroidBound.pMax[axis] == centroidBound.pMin[axis]) {
			Shapes shapes;
			for (int i = start; i < end; ++i) {
				size_t idx = shapeInfos[i].idx;
				Shape* shape = geometries[idx];
				shapes.push_back(shape);
			}
			node->InitAsLeaf(shapes, boundAll);
			return node;
		}
		if (count <= 2) {
			mid = (start + end) / 2;
		}
		else {
			constexpr int nBuckets = 12;
			BVHBucket buckets[nBuckets];
			for (int i = start; i < end; ++i) {
				int b = nBuckets * centroidBound.Offset(shapeInfos[i].centroid)[axis];
				if (b == nBuckets) b = nBuckets - 1;
				Assert(b >= 0 && b < nBuckets);
				buckets[b].count++;
				buckets[b].bound = Union(buckets[b].bound, shapeInfos[i].bound);
			}
			float cost[nBuckets - 1];
			for (int i = 0; i < nBuckets - 1; ++i) {
				BBox b0, b1;
				int count0 = 0, count1 = 0;
				for (int j = 0; j <= i; ++j) {
					b0 = Union(b0, buckets[j].bound);
					count0 += buckets[j].count;
				}
				for (int j = i + 1; j < nBuckets; ++j) {
					b1 = Union(b1, buckets[j].bound);
					count1 += buckets[j].count;
				}
				cost[i] = 1 +
					(count0 * b0.SurfaceArea() +
						count1 * b1.SurfaceArea()) /
					boundAll.SurfaceArea();
			}

			// Find bucket to split at that minimizes SAH metric
			float minCost = cost[0];
			int minCostSplitBucket = 0;
			for (int i = 1; i < nBuckets - 1; ++i) {
				if (cost[i] < minCost) {
					minCost = cost[i];
					minCostSplitBucket = i;
				}
			}

			// Either create leaf or split primitives at selected SAH
			// bucket
			float leafCost = count;
			int maxPrimsInNode = 4;
			if (count > maxPrimsInNode || minCost < leafCost) {
				BVHShapeInfo *pmid = std::partition(
					&shapeInfos[start], &shapeInfos[end - 1] + 1,
					[=](const BVHShapeInfo& pi) {
					int b = nBuckets *
						centroidBound.Offset(pi.centroid)[axis];
					if (b == nBuckets) b = nBuckets - 1;
					Assert(b >= 0 && b < nBuckets);
					return b <= minCostSplitBucket;
				});
				mid = pmid - &shapeInfos[0];
			}
			else {
				Shapes shapes;
				for (int i = start; i < end; ++i) {
					int idx = shapeInfos[i].idx;
					Shape* shape = geometries[idx];
					shapes.push_back(shape);
				}
				node->InitAsLeaf(shapes, boundAll);
				return node;
			}
		}
		auto left = recursiveBuild(shapeInfos, start, mid);
		auto right = recursiveBuild(shapeInfos, mid, end);
		node->InitAsInterior(axis, left, right);
		return node;
	}

	int BVHTree::Intersect(Ray& ray, IntersectResult* result) {
		if (!root) return 0;
		Vector3dF invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
		int dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };
		std::list<BVHNode*> stack;
		stack.push_back(root);
		while (stack.size() > 0) {
			BVHNode* node = stack.back();
			stack.pop_back();
			// Check ray against BVH node
			float hit0, hit1;
			bool isHit = node->bound.Intersect(ray, &hit0, &hit1);
			if (isHit && ray.tMax > hit0) {
				if (node->shapes.size() > 0) { // leaf
					for (auto shape : node->shapes) {
						IntersectResult resultTmp;
						logDebug("check shape, id:%d \n", shape->id);
						shape->Intersect(ray, &resultTmp);
						if (!result->geometry ||
							(resultTmp.geometry && resultTmp.distance < result->distance)) {
							*result = resultTmp;
						}
					}
					if (stack.size() == 0) break;
				}
				else { // interior
					if (dirIsNeg[node->splitAxis]) {
						stack.push_back(node->right);
						stack.push_back(node->left);
					}
					else {
						stack.push_back(node->left);
						stack.push_back(node->right);
					}
				}
			}
			else {
				if(stack.size() == 0) break;
			}
		}
		return 0;
	}

}