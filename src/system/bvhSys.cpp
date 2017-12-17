#include "stdafx.h"
#include "system/bvhSys.hpp"
#include "com/geometry.hpp"
#include "com/spatialData.hpp"
#include "com/shader.hpp"
#include "event/shaderEvent.hpp"
#include "event/bufferEvent.hpp"

using namespace std;


namespace renderer {


	struct BucketInfo {
		int count = 0;
		BBox bounds;
	};


	// BVHAccel Utility Functions
	inline uint32_t LeftShift3(uint32_t x) {
		Assert(x <= (1 << 10));
		if (x == (1 << 10)) --x;
		x = (x | (x << 16)) & 0b00000011000000000000000011111111;
		// x = ---- --98 ---- ---- ---- ---- 7654 3210
		x = (x | (x << 8)) & 0b00000011000000001111000000001111;
		// x = ---- --98 ---- ---- 7654 ---- ---- 3210
		x = (x | (x << 4)) & 0b00000011000011000011000011000011;
		// x = ---- --98 ---- 76-- --54 ---- 32-- --10
		x = (x | (x << 2)) & 0b00001001001001001001001001001001;
		// x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
		return x;
	}

	inline uint32_t EncodeMorton3(const Vector3dF &v) {
		Assert(v.x >= 0 && v.x <= (1 << 10));
		Assert(v.y >= 0 && v.y <= (1 << 10));
		Assert(v.z >= 0 && v.z <= (1 << 10));
		return (LeftShift3(v.z) << 2) | (LeftShift3(v.y) << 1) | LeftShift3(v.x);
	}

	static void RadixSort(std::vector<MortonPrimitive> *v) {
		std::vector<MortonPrimitive> tempVector(v->size());
		constexpr int bitsPerPass = 6;
		constexpr int nBits = 30;
		Assert((nBits % bitsPerPass) == 0);
		constexpr int nPasses = nBits / bitsPerPass;
		for (int pass = 0; pass < nPasses; ++pass) {
			// Perform one pass of radix sort, sorting _bitsPerPass_ bits
			int lowBit = pass * bitsPerPass;

			// Set in and out vector pointers for radix sort pass
			std::vector<MortonPrimitive> &in = (pass & 1) ? tempVector : *v;
			std::vector<MortonPrimitive> &out = (pass & 1) ? *v : tempVector;

			// Count number of zero bits in array for current radix sort bit
			constexpr int nBuckets = 1 << bitsPerPass;
			int bucketCount[nBuckets] = { 0 };
			constexpr int bitMask = (1 << bitsPerPass) - 1;
			for (const MortonPrimitive &mp : in) {
				int bucket = (mp.mortonCode >> lowBit) & bitMask;
				Assert(bucket >= 0 && bucket < nBuckets);
				++bucketCount[bucket];
			}

			// Compute starting index in output array for each bucket
			int outIndex[nBuckets];
			outIndex[0] = 0;
			for (int i = 1; i < nBuckets; ++i)
				outIndex[i] = outIndex[i - 1] + bucketCount[i - 1];

			// Store sorted values in output array
			for (const MortonPrimitive &mp : in) {
				int bucket = (mp.mortonCode >> lowBit) & bitMask;
				out[outIndex[bucket]++] = mp;
			}
		}
		// Copy final result from _tempVector_, if needed
		if (nPasses & 1) std::swap(*v, tempVector);
	}

	void BVHSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("BVHSystem init\n");
		evtMgr.on<CreateBVHEvent>(*this);
		evtMgr.on<DebugDrawBVHEvent>(*this);
	}

	void BVHSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}

	void BVHSystem::receive(const DebugDrawBVHEvent &evt) {
		auto bvhAccel = evt.objBVH.component<BVHAccel>();
		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
		auto matSetCom = m_objMgr->getSingletonComponent<MaterialSet>();
		auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
		Shader shader = spSetCom->getShader("wireframe");
		shader.use();
		m_evtMgr->emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
		LinearBVHNode *nodes = bvhAccel->nodes;
		int toVisitOffset = 0, currentNodeIndex = 0;
		int nodesToVisit[64];
		// Turn on wireframe mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		while (true) {
			const LinearBVHNode *node = &nodes[currentNodeIndex];
			if (node->nPrimitives > 0) {
				// draw bounds
				auto worldBound = node->bounds;
				auto pos = (worldBound.pMax + worldBound.pMin) * 0.5f;
				auto len = worldBound.pMax - worldBound.pMin;
				Matrix4x4 T = Translate<Matrix4x4>(pos);
				Matrix4x4 S = Scale<Matrix4x4>(len);
				shader.setMatrix4f("modelMat", T * S);
				m_evtMgr->emit<DrawMeshBufferEvent>("box", 0);
				
				//for (int i = 0; i < node->nPrimitives; ++i) {
				//	auto objID = bvhAccel->primitives[node->primitivesOffset + i];
				//}
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			} else {
				nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
				// currentNodeIndex = node->secondChildOffset;
				nodesToVisit[toVisitOffset++] = node->secondChildOffset;
				currentNodeIndex = currentNodeIndex + 1;
			}
		};
		// Turn off wireframe mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		/*
		LinearBVHNode *nodes = bvhAccel->nodes;
		if (!nodes) return;
		bool hit = false;
		int toVisitOffset = 0, currentNodeIndex = 0;
		int nodesToVisit[64];
		while (true) {
		const LinearBVHNode *node = &nodes[currentNodeIndex];
		// Check ray against BVH node
		if (true) {
		if (node->nPrimitives > 0) {
		// Intersect ray with primitives in leaf BVH node
		for (int i = 0; i < node->nPrimitives; ++i)
		if (true)
		hit = true;
		if (toVisitOffset == 0) break;
		currentNodeIndex = nodesToVisit[--toVisitOffset];
		}
		else {
		// Put far BVH node on _nodesToVisit_ stack, advance to near
		// node
		if (true) {
		nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
		currentNodeIndex = node->secondChildOffset;
		}
		else {
		nodesToVisit[toVisitOffset++] = node->secondChildOffset;
		currentNodeIndex = currentNodeIndex + 1;
		}
		}
		}
		else {
		if (toVisitOffset == 0) break;
		currentNodeIndex = nodesToVisit[--toVisitOffset];
		}
		}*/
	}

	void BVHSystem::receive(const CreateBVHEvent &evt) {
		Object objScene = evt.objScene;
		Object objBVH = evt.objBVH;
		auto bvhAccel = objBVH.addComponent<BVHAccel>();

		std::vector<ObjectID> prims;
		std::function<void(Object objScene)> filterFunc;
		filterFunc = [&](Object objScene) {
			if (objScene.hasComponent<SpatialData>() && objScene.hasComponent<MeshRef>()) {
				ObjectID objID = objScene.ID();
				prims.push_back(objID);
			}
			auto sgNode = objScene.component<SceneGraphNode>();
			for (auto childObjID : sgNode->children) {
				Object childObj = m_objMgr->get(childObjID);
				filterFunc(childObj);
			}
		}; 
		filterFunc(objScene);
		CreateBVHAccel(bvhAccel, prims,"", 0);
	}

	// BVHAccel Method Definitions
	void BVHSystem::CreateBVHAccel(ComponentHandle<BVHAccel> bvhAccel, const std::vector<ObjectID> &p,
		int maxPrimsInNode, BVHAccel::SplitMethod splitMethod) {
		bvhAccel->maxPrimsInNode = std::min(255, maxPrimsInNode);
		bvhAccel->splitMethod = splitMethod;
		bvhAccel->primitives = p;
		if (bvhAccel->primitives.size() == 0) return;
		// Build BVH from _primitives_

		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();

		// Initialize _primitiveInfo_ array for primitives
		std::vector<BVHPrimitiveInfo> primitiveInfo(bvhAccel->primitives.size());
		for (size_t i = 0; i < bvhAccel->primitives.size(); ++i) {
			ObjectID objID = bvhAccel->primitives[i];
			Object obj = m_objMgr->get(objID);
			auto meshRef = obj.component<MeshRef>();
			auto spatialData = obj.component<SpatialData>();
			Mesh& mesh = meshSet->getMesh(meshRef->meshID);
			auto bound = mesh.Bound();
			auto worldBound = (spatialData->o2w)(bound);
			primitiveInfo[i] = { i, worldBound };
		}

		int totalNodes = 0;
		std::vector<ObjectID> orderedPrims;
		orderedPrims.reserve(bvhAccel->primitives.size());
		BVHBuildNode *root;
		if (splitMethod == BVHAccel::SplitMethod::HLBVH)
			root = HLBVHBuild(primitiveInfo, &totalNodes, orderedPrims);
		else
			root = recursiveBuild(bvhAccel, primitiveInfo, 0, bvhAccel->primitives.size(),
				&totalNodes, orderedPrims);
		bvhAccel->primitives.swap(orderedPrims);
		Info("BVH created with %d nodes for %d primitives (%.2f MB)", totalNodes,
			(int)bvhAccel->primitives.size(),
			float(totalNodes * sizeof(LinearBVHNode)) / (1024.f * 1024.f));

		// Compute representation of depth-first traversal of BVH tree
		bvhAccel->nodes = AllocAligned<LinearBVHNode>(totalNodes);
		int offset = 0;
		flattenBVHTree(bvhAccel, root, &offset);
		Assert(offset == totalNodes);
	}


	BVHBuildNode *BVHSystem::recursiveBuild(
		ComponentHandle<BVHAccel> bvhAccel, 
		std::vector<BVHPrimitiveInfo> &primitiveInfo, 
		int start,
		int end, 
		int *totalNodes,
		std::vector<ObjectID> &orderedPrims) {
		Assert(start != end);
		BVHBuildNode *node = GetPool<BVHBuildNode>()->newElement(BVHBuildNode());
		(*totalNodes)++;
		// Compute bounds of all primitives in BVH node
		BBox bounds;
		for (int i = start; i < end; ++i)
			bounds = Union(bounds, primitiveInfo[i].bounds);
		int nPrimitives = end - start;
		if (nPrimitives == 1) {
			// Create leaf _BVHBuildNode_
			int firstPrimOffset = orderedPrims.size();
			for (int i = start; i < end; ++i) {
				int primNum = primitiveInfo[i].primitiveNumber;
				orderedPrims.push_back(bvhAccel->primitives[primNum]);
			}
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else {
			// Compute bound of primitive centroids, choose split dimension _dim_
			BBox centroidBounds;
			for (int i = start; i < end; ++i)
				centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
			Axis dim = centroidBounds.MaximumExtent();

			// Partition primitives into two sets and build children
			int mid = (start + end) / 2;
			if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim]) {
				// Create leaf _BVHBuildNode_
				int firstPrimOffset = orderedPrims.size();
				for (int i = start; i < end; ++i) {
					int primNum = primitiveInfo[i].primitiveNumber;
					orderedPrims.push_back(bvhAccel->primitives[primNum]);
				}
				node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
				return node;
			}
			else {
				// Partition primitives based on _splitMethod_
				switch (bvhAccel->splitMethod) {
				case BVHAccel::SplitMethod::Middle: {
					// Partition primitives through node's midpoint
					float pmid =
						(centroidBounds.pMin[dim] + centroidBounds.pMax[dim]) / 2;
					BVHPrimitiveInfo *midPtr = std::partition(
						&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
						[dim, pmid](const BVHPrimitiveInfo &pi) {
						return pi.centroid[dim] < pmid;
					});
					mid = midPtr - &primitiveInfo[0];
					// For lots of prims with large overlapping bounding boxes, this
					// may fail to partition; in that case don't break and fall
					// through
					// to EqualCounts.
					if (mid != start && mid != end) break;
				}
				case BVHAccel::SplitMethod::EqualCounts: {
					// Partition primitives into equally-sized subsets
					mid = (start + end) / 2;
					std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
						&primitiveInfo[end - 1] + 1,
						[dim](const BVHPrimitiveInfo &a,
							const BVHPrimitiveInfo &b) {
						return a.centroid[dim] < b.centroid[dim];
					});
					break;
				}
				case BVHAccel::SplitMethod::SAH:
				default: {
					// Partition primitives using approximate SAH
					if (nPrimitives <= 2) {
						// Partition primitives into equally-sized subsets
						mid = (start + end) / 2;
						std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
							&primitiveInfo[end - 1] + 1,
							[dim](const BVHPrimitiveInfo &a,
								const BVHPrimitiveInfo &b) {
							return a.centroid[dim] <
								b.centroid[dim];
						});
					}
					else {
						// Allocate _BucketInfo_ for SAH partition buckets
						constexpr int nBuckets = 12;
						BucketInfo buckets[nBuckets];

						// Initialize _BucketInfo_ for SAH partition buckets
						for (int i = start; i < end; ++i) {
							int b = nBuckets *
								centroidBounds.Offset(
									primitiveInfo[i].centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							Assert(b >= 0 && b < nBuckets);
							buckets[b].count++;
							buckets[b].bounds =
								Union(buckets[b].bounds, primitiveInfo[i].bounds);
						}

						// Compute costs for splitting after each bucket
						float cost[nBuckets - 1];
						for (int i = 0; i < nBuckets - 1; ++i) {
							BBox b0, b1;
							int count0 = 0, count1 = 0;
							for (int j = 0; j <= i; ++j) {
								b0 = Union(b0, buckets[j].bounds);
								count0 += buckets[j].count;
							}
							for (int j = i + 1; j < nBuckets; ++j) {
								b1 = Union(b1, buckets[j].bounds);
								count1 += buckets[j].count;
							}
							cost[i] = 1 +
								(count0 * b0.SurfaceArea() +
									count1 * b1.SurfaceArea()) /
								bounds.SurfaceArea();
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
						float leafCost = nPrimitives;
						if (nPrimitives > bvhAccel->maxPrimsInNode || minCost < leafCost) {
							BVHPrimitiveInfo *pmid = std::partition(
								&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
								[=](const BVHPrimitiveInfo &pi) {
								int b = nBuckets *
									centroidBounds.Offset(pi.centroid)[dim];
								if (b == nBuckets) b = nBuckets - 1;
								Assert(b >= 0 && b < nBuckets);
								return b <= minCostSplitBucket;
							});
							mid = pmid - &primitiveInfo[0];
						}
						else {
							// Create leaf _BVHBuildNode_
							int firstPrimOffset = orderedPrims.size();
							for (int i = start; i < end; ++i) {
								int primNum = primitiveInfo[i].primitiveNumber;
								orderedPrims.push_back(bvhAccel->primitives[primNum]);
							}
							node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
							return node;
						}
					}
					break;
				}
				}
				node->InitInterior(dim,
					recursiveBuild(bvhAccel, primitiveInfo, start, mid,
						totalNodes, orderedPrims),
					recursiveBuild(bvhAccel, primitiveInfo, mid, end,
						totalNodes, orderedPrims));
			}
		}
		return node;
	}

	BVHBuildNode *BVHSystem::HLBVHBuild(const std::vector<BVHPrimitiveInfo> &primitiveInfo,
		int *totalNodes,
		std::vector<ObjectID> &orderedPrims) const {
		return nullptr;
		/*
		// Compute bounding box of all primitive centroids
		BBox bounds;
		for (const BVHPrimitiveInfo &pi : primitiveInfo)
		bounds = Union(bounds, pi.centroid);

		// Compute Morton indices of primitives
		std::vector<MortonPrimitive> mortonPrims(primitiveInfo.size());
		ParallelFor([&](int i) {
		// Initialize _mortonPrims[i]_ for _i_th primitive
		constexpr int mortonBits = 10;
		constexpr int mortonScale = 1 << mortonBits;
		mortonPrims[i].primitiveIndex = primitiveInfo[i].primitiveNumber;
		Vector3dF centroidOffset = bounds.Offset(primitiveInfo[i].centroid);
		mortonPrims[i].mortonCode = EncodeMorton3(centroidOffset * mortonScale);
		}, primitiveInfo.size(), 512);

		// Radix sort primitive Morton indices
		RadixSort(&mortonPrims);

		// Create LBVH treelets at bottom of BVH

		// Find intervals of primitives for each treelet
		std::vector<LBVHTreelet> treeletsToBuild;
		for (int start = 0, end = 1; end <= (int)mortonPrims.size(); ++end) {
		uint32_t mask = 0b00111111111111000000000000000000;
		if (end == (int)mortonPrims.size() ||
		((mortonPrims[start].mortonCode & mask) !=
		(mortonPrims[end].mortonCode & mask))) {
		// Add entry to _treeletsToBuild_ for this treelet
		int nPrimitives = end - start;
		int maxBVHNodes = 2 * nPrimitives;
		BVHBuildNode *nodes = GetPool<BVHBuildNode>()->newElement(maxBVHNodes, false);
		treeletsToBuild.push_back({ start, nPrimitives, nodes });
		start = end;
		}
		}

		// Create LBVHs for treelets in parallel
		std::atomic<int> atomicTotal(0), orderedPrimsOffset(0);
		orderedPrims.resize(primitives.size());
		ParallelFor([&](int i) {
		// Generate _i_th LBVH treelet
		int nodesCreated = 0;
		const int firstBitIndex = 29 - 12;
		LBVHTreelet &tr = treeletsToBuild[i];
		tr.buildNodes =
		emitLBVH(tr.buildNodes, primitiveInfo, &mortonPrims[tr.startIndex],
		tr.nPrimitives, &nodesCreated, orderedPrims,
		&orderedPrimsOffset, firstBitIndex);
		atomicTotal += nodesCreated;
		}, treeletsToBuild.size());
		*totalNodes = atomicTotal;

		// Create and return SAH BVH from LBVH treelets
		std::vector<BVHBuildNode *> finishedTreelets;
		finishedTreelets.reserve(treeletsToBuild.size());
		for (LBVHTreelet &treelet : treeletsToBuild)
		finishedTreelets.push_back(treelet.buildNodes);
		return buildUpperSAH(finishedTreelets, 0, finishedTreelets.size(),
		totalNodes);
		*/
	}

	BVHBuildNode *BVHSystem::emitLBVH(ComponentHandle<BVHAccel> bvhAccel,
		BVHBuildNode *&buildNodes,
		const std::vector<BVHPrimitiveInfo> &primitiveInfo,
		MortonPrimitive *mortonPrims, int nPrimitives, int *totalNodes,
		std::vector<ObjectID> &orderedPrims,
		std::atomic<int> *orderedPrimsOffset, int bitIndex) const {
		Assert(nPrimitives > 0);
		if (bitIndex == -1 || nPrimitives < bvhAccel->maxPrimsInNode) {
			// Create and return leaf node of LBVH treelet
			(*totalNodes)++;
			BVHBuildNode *node = buildNodes++;
			BBox bounds;
			int firstPrimOffset = orderedPrimsOffset->fetch_add(nPrimitives);
			for (int i = 0; i < nPrimitives; ++i) {
				int primitiveIndex = mortonPrims[i].primitiveIndex;
				orderedPrims[firstPrimOffset + i] = bvhAccel->primitives[primitiveIndex];
				bounds = Union(bounds, primitiveInfo[primitiveIndex].bounds);
			}
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else {
			int mask = 1 << bitIndex;
			// Advance to next subtree level if there's no LBVH split for this bit
			if ((mortonPrims[0].mortonCode & mask) ==
				(mortonPrims[nPrimitives - 1].mortonCode & mask))
				return emitLBVH(bvhAccel, buildNodes, primitiveInfo, mortonPrims, nPrimitives,
					totalNodes, orderedPrims, orderedPrimsOffset,
					bitIndex - 1);

			// Find LBVH split point for this dimension
			int searchStart = 0, searchEnd = nPrimitives - 1;
			while (searchStart + 1 != searchEnd) {
				Assert(searchStart != searchEnd);
				int mid = (searchStart + searchEnd) / 2;
				if ((mortonPrims[searchStart].mortonCode & mask) ==
					(mortonPrims[mid].mortonCode & mask))
					searchStart = mid;
				else {
					Assert((mortonPrims[mid].mortonCode & mask) ==
						(mortonPrims[searchEnd].mortonCode & mask));
					searchEnd = mid;
				}
			}
			int splitOffset = searchEnd;
			Assert(splitOffset <= nPrimitives - 1);
			Assert((mortonPrims[splitOffset - 1].mortonCode & mask) !=
				(mortonPrims[splitOffset].mortonCode & mask));

			// Create and return interior LBVH node
			(*totalNodes)++;
			BVHBuildNode *node = buildNodes++;
			BVHBuildNode *lbvh[2] = {
				emitLBVH(bvhAccel, buildNodes, primitiveInfo, mortonPrims, splitOffset,
				totalNodes, orderedPrims, orderedPrimsOffset,
				bitIndex - 1),
				emitLBVH(bvhAccel, buildNodes, primitiveInfo, &mortonPrims[splitOffset],
				nPrimitives - splitOffset, totalNodes, orderedPrims,
				orderedPrimsOffset, bitIndex - 1) };
			Axis axis = static_cast<Axis>(bitIndex % 3);
			node->InitInterior(axis, lbvh[0], lbvh[1]);
			return node;
		}
	}

	BVHBuildNode *BVHSystem::buildUpperSAH(std::vector<BVHBuildNode *> &treeletRoots,
		int start, int end,
		int *totalNodes) const {
		Assert(start < end);
		int nNodes = end - start;
		if (nNodes == 1) return treeletRoots[start];
		(*totalNodes)++;
		BVHBuildNode *node = GetPool<BVHBuildNode>()->newElement();

		// Compute bounds of all nodes under this HLBVH node
		BBox bounds;
		for (int i = start; i < end; ++i)
			bounds = Union(bounds, treeletRoots[i]->bounds);

		// Compute bound of HLBVH node centroids, choose split dimension _dim_
		BBox centroidBounds;
		for (int i = start; i < end; ++i) {
			Vector3dF centroid =
				(treeletRoots[i]->bounds.pMin + treeletRoots[i]->bounds.pMax) *
				0.5f;
			centroidBounds = Union(centroidBounds, centroid);
		}
		Axis dim = centroidBounds.MaximumExtent();
		// FIXME: if this hits, what do we need to do?
		// Make sure the SAH split below does something... ?
		Assert(centroidBounds.pMax[dim] != centroidBounds.pMin[dim]);

		// Allocate _BucketInfo_ for SAH partition buckets
		constexpr int nBuckets = 12;

		BucketInfo buckets[nBuckets];

		// Initialize _BucketInfo_ for HLBVH SAH partition buckets
		for (int i = start; i < end; ++i) {
			float centroid = (treeletRoots[i]->bounds.pMin[dim] +
				treeletRoots[i]->bounds.pMax[dim]) *
				0.5f;
			int b =
				nBuckets * ((centroid - centroidBounds.pMin[dim]) /
				(centroidBounds.pMax[dim] - centroidBounds.pMin[dim]));
			if (b == nBuckets) b = nBuckets - 1;
			Assert(b >= 0 && b < nBuckets);
			buckets[b].count++;
			buckets[b].bounds = Union(buckets[b].bounds, treeletRoots[i]->bounds);
		}

		// Compute costs for splitting after each bucket
		float cost[nBuckets - 1];
		for (int i = 0; i < nBuckets - 1; ++i) {
			BBox b0, b1;
			int count0 = 0, count1 = 0;
			for (int j = 0; j <= i; ++j) {
				b0 = Union(b0, buckets[j].bounds);
				count0 += buckets[j].count;
			}
			for (int j = i + 1; j < nBuckets; ++j) {
				b1 = Union(b1, buckets[j].bounds);
				count1 += buckets[j].count;
			}
			cost[i] = .125f +
				(count0 * b0.SurfaceArea() + count1 * b1.SurfaceArea()) /
				bounds.SurfaceArea();
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

		// Split nodes and create interior HLBVH SAH node
		BVHBuildNode **pmid = std::partition(
			&treeletRoots[start], &treeletRoots[end - 1] + 1,
			[=](const BVHBuildNode *node) {
			float centroid =
				(node->bounds.pMin[dim] + node->bounds.pMax[dim]) * 0.5f;
			int b = nBuckets *
				((centroid - centroidBounds.pMin[dim]) /
				(centroidBounds.pMax[dim] - centroidBounds.pMin[dim]));
			if (b == nBuckets) b = nBuckets - 1;
			Assert(b >= 0 && b < nBuckets);
			return b <= minCostSplitBucket;
		});
		int mid = pmid - &treeletRoots[0];
		Assert(mid > start && mid < end);
		node->InitInterior(
			dim, buildUpperSAH(treeletRoots, start, mid, totalNodes),
			buildUpperSAH(treeletRoots, mid, end, totalNodes));
		return node;
	}

	int BVHSystem::flattenBVHTree(ComponentHandle<BVHAccel> bvhAccel, BVHBuildNode *node, int *offset) {
		LinearBVHNode *linearNode = &bvhAccel->nodes[*offset];
		linearNode->bounds = node->bounds;
		int myOffset = (*offset)++;
		if (node->nPrimitives > 0) {
			Assert(!node->children[0] && !node->children[1]);
			Assert(node->nPrimitives < 65536);
			linearNode->primitivesOffset = node->firstPrimOffset;
			linearNode->nPrimitives = node->nPrimitives;
		}
		else {
			// Create interior flattened BVH node
			linearNode->axis = node->splitAxis;
			linearNode->nPrimitives = 0;
			flattenBVHTree(bvhAccel, node->children[0], offset);
			linearNode->secondChildOffset =
				flattenBVHTree(bvhAccel, node->children[1], offset);
		}
		return myOffset;
	}


	void BVHSystem::CreateBVHAccel(ComponentHandle<BVHAccel> bvhAccel,
		const std::vector<ObjectID> &prims, std::string splitMethodName, int maxPrimsInNode) {
		if (splitMethodName == "") {
			splitMethodName = "sah";
		}
		if (!maxPrimsInNode) {
			maxPrimsInNode = 4;
		}
		BVHAccel::SplitMethod splitMethod;
		if (splitMethodName == "sah")
			splitMethod = BVHAccel::SplitMethod::SAH;
		else if (splitMethodName == "hlbvh")
			splitMethod = BVHAccel::SplitMethod::HLBVH;
		else if (splitMethodName == "middle")
			splitMethod = BVHAccel::SplitMethod::Middle;
		else if (splitMethodName == "equal")
			splitMethod = BVHAccel::SplitMethod::EqualCounts;
		else {
			Warning("BVH split method \"%s\" unknown.  Using \"sah\".",
				splitMethodName.c_str());
			splitMethod = BVHAccel::SplitMethod::SAH;
		}
		CreateBVHAccel(bvhAccel, prims, maxPrimsInNode, splitMethod);
	}

};