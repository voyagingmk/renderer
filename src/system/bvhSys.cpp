#include "stdafx.h"
#include "system/bvhSys.hpp"
#include "com/geometry.hpp"
#include "com/spatialData.hpp"
#include "com/shader.hpp"
#include "event/shaderEvent.hpp"
#include "event/bufferEvent.hpp"
#include "com/miscCom.hpp"
#include "utils/parallel.hpp"


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

	static void RadixSort(std::vector<MortonObj> *v) {
		std::vector<MortonObj> tempVector(v->size());
		constexpr int bitsPerPass = 6;
		constexpr int nBits = 30;
		Assert((nBits % bitsPerPass) == 0);
		constexpr int nPasses = nBits / bitsPerPass;
		for (int pass = 0; pass < nPasses; ++pass) {
			// Perform one pass of radix sort, sorting _bitsPerPass_ bits
			int lowBit = pass * bitsPerPass;

			// Set in and out vector pointers for radix sort pass
			std::vector<MortonObj> &in = (pass & 1) ? tempVector : *v;
			std::vector<MortonObj> &out = (pass & 1) ? *v : tempVector;

			// Count number of zero bits in array for current radix sort bit
			constexpr int nBuckets = 1 << bitsPerPass;
			int bucketCount[nBuckets] = { 0 };
			constexpr int bitMask = (1 << bitsPerPass) - 1;
			for (const MortonObj &mp : in) {
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
			for (const MortonObj &mp : in) {
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
		evtMgr.on<ComponentRemovedEvent<BVHAccel>>(*this);
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
		int nodesDepth[64];
		nodesDepth[0] = 0;
		// Turn on wireframe mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		int depth = 0;
		while (true) {
			const LinearBVHNode *node = &nodes[currentNodeIndex];
			// draw bounds
			auto worldBound = node->bounds;
			auto pos = (worldBound.pMax + worldBound.pMin) * 0.5f;
			auto len = worldBound.pMax - worldBound.pMin;
			Matrix4x4 T = Translate<Matrix4x4>(pos);
			Matrix4x4 S = Scale<Matrix4x4>(len);
			shader.setMatrix4f("modelMat", T * S);
			shader.set3f("wireColor", Vector3dF{ 1.0f, 1.0f, 1.0f} * ( 0.2f + (float)depth / 20.0f));
			auto gSettingCom = m_objMgr->getSingletonComponent<GlobalSettingCom>();
			int d = gSettingCom->getValue("bvhDepth");
			if (d == depth) {
				m_evtMgr->emit<DrawMeshBufferEvent>("wfbox", 0);
			}
			if (node->nObjs > 0) {
				//for (int i = 0; i < node->nObjs; ++i) {
				//	auto objID = bvhAccel->objs[node->objsOffset + i];
				//}
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
				depth = nodesDepth[toVisitOffset];
			} else {
				depth++;
				nodesDepth[toVisitOffset] = depth;
				nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
				// currentNodeIndex = node->secondChildOffset;
				nodesDepth[toVisitOffset] = depth;
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
		if (node->nObjs > 0) {
		// Intersect ray with objs in leaf BVH node
		for (int i = 0; i < node->nObjs; ++i)
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
	

	void BVHSystem::receive(const ComponentRemovedEvent<BVHAccel> &evt) {
		if (evt.component->nodes) {
			FreeAligned(evt.component->nodes);
		}
	}

	void BVHSystem::receive(const CreateBVHEvent &evt) {
		Object objScene = evt.objScene;
		Object objBVH = evt.objBVH;
		if (objBVH.hasComponent<BVHAccel>()) {
			return;
		}
		auto bvhAccel = objBVH.addComponent<BVHAccel>();

		std::vector<ObjectID> objs;
		std::function<void(Object objScene)> filterFunc;
		filterFunc = [&](Object objScene) {
			if (objScene.hasComponent<SpatialData>() && objScene.hasComponent<MeshRef>()) {
				ObjectID objID = objScene.ID();
				objs.push_back(objID);
			}
			auto sgNode = objScene.component<SceneGraphNode>();
			for (auto childObjID : sgNode->children) {
				Object childObj = m_objMgr->get(childObjID);
				filterFunc(childObj);
			}
		}; 
		filterFunc(objScene);
		CreateBVHAccel(bvhAccel, objs, "", 0);
	}


	void BVHSystem::CreateBVHAccel(ComponentHandle<BVHAccel> bvhAccel, const std::vector<ObjectID> &p,
		int maxObjsInNode, BVHAccel::SplitMethod splitMethod) {
		bvhAccel->maxObjsInNode = std::min(255, maxObjsInNode);
		bvhAccel->splitMethod = splitMethod;
		bvhAccel->objs = p;
		if (bvhAccel->objs.size() == 0) return;
		// Build BVH from _objs_

		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();

		std::vector<BVHObjInfo> objInfo(bvhAccel->objs.size());
		for (size_t i = 0; i < bvhAccel->objs.size(); ++i) {
			ObjectID objID = bvhAccel->objs[i];
			Object obj = m_objMgr->get(objID);
			auto meshRef = obj.component<MeshRef>();
			auto spatialData = obj.component<SpatialData>();
			Mesh& mesh = meshSet->getMesh(meshRef->meshID);
			auto bound = mesh.Bound();
			auto worldBound = (spatialData->o2w)(bound);
			objInfo[i] = { i, worldBound };
		}

		int totalNodes = 0;
		std::vector<ObjectID> orderedObjs;
		orderedObjs.reserve(bvhAccel->objs.size());
		BVHBuildNode *root;
		if (splitMethod == BVHAccel::SplitMethod::HLBVH)
			root = HLBVHBuild(bvhAccel, objInfo, &totalNodes, orderedObjs);
		else
			root = recursiveBuild(bvhAccel, objInfo, 0, bvhAccel->objs.size(),
				&totalNodes, orderedObjs);
		bvhAccel->objs.swap(orderedObjs);
		Info("BVH created with %d nodes for %d objs (%.2f MB)", totalNodes,
			(int)bvhAccel->objs.size(),
			float(totalNodes * sizeof(LinearBVHNode)) / (1024.f * 1024.f));

		// Compute representation of depth-first traversal of BVH tree
		bvhAccel->nodes = AllocAligned<LinearBVHNode>(totalNodes);
		int offset = 0;
		flattenBVHTree(bvhAccel, root, &offset);
		Assert(offset == totalNodes);
	}


	BVHBuildNode *BVHSystem::recursiveBuild(
		ComponentHandle<BVHAccel> bvhAccel, 
		std::vector<BVHObjInfo> &objInfo, 
		int start,
		int end, 
		int *totalNodes,
		std::vector<ObjectID> &orderedObjs) {
		Assert(start != end);
		BVHBuildNode *node = GetPool<BVHBuildNode>()->newElement();
		(*totalNodes)++;
		// Compute bounds of all objs in BVH node
		BBox bounds;
		for (int i = start; i < end; ++i)
			bounds = Union(bounds, objInfo[i].bounds);
		int nObjs = end - start;
		if (nObjs == 1) {
			// Create leaf _BVHBuildNode_
			int firstPrimOffset = orderedObjs.size();
			for (int i = start; i < end; ++i) {
				int primNum = objInfo[i].objNumber;
				orderedObjs.push_back(bvhAccel->objs[primNum]);
			}
			InitLeaf(node, firstPrimOffset, nObjs, bounds);
			return node;
		}
		else {
			// Compute bound of obj centroids, choose split dimension _dim_
			BBox centroidBounds;
			for (int i = start; i < end; ++i)
				centroidBounds = Union(centroidBounds, objInfo[i].centroid);
			Axis dim = centroidBounds.MaximumExtent();

			// Partition objs into two sets and build children
			int mid = (start + end) / 2;
			if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim]) {
				// Create leaf _BVHBuildNode_
				int firstPrimOffset = orderedObjs.size();
				for (int i = start; i < end; ++i) {
					int primNum = objInfo[i].objNumber;
					orderedObjs.push_back(bvhAccel->objs[primNum]);
				}
				InitLeaf(node, firstPrimOffset, nObjs, bounds);
				return node;
			}
			else {
				// Partition objs based on _splitMethod_
				switch (bvhAccel->splitMethod) {
				case BVHAccel::SplitMethod::Middle: {
					// Partition objs through node's midpoint
					float pmid =
						(centroidBounds.pMin[dim] + centroidBounds.pMax[dim]) / 2;
					BVHObjInfo *midPtr = std::partition(
						&objInfo[start], &objInfo[end - 1] + 1,
						[dim, pmid](const BVHObjInfo &pi) {
						return pi.centroid[dim] < pmid;
					});
					mid = midPtr - &objInfo[0];
					// For lots of objs with large overlapping bounding boxes, this
					// may fail to partition; in that case don't break and fall
					// through
					// to EqualCounts.
					if (mid != start && mid != end) break;
				}
				case BVHAccel::SplitMethod::EqualCounts: {
					// Partition objs into equally-sized subsets
					mid = (start + end) / 2;
					std::nth_element(&objInfo[start], &objInfo[mid],
						&objInfo[end - 1] + 1,
						[dim](const BVHObjInfo &a,
							const BVHObjInfo &b) {
						return a.centroid[dim] < b.centroid[dim];
					});
					break;
				}
				case BVHAccel::SplitMethod::SAH:
				default: {
					// Partition objs using approximate SAH
					if (nObjs <= 2) {
						// Partition objs into equally-sized subsets
						mid = (start + end) / 2;
						std::nth_element(&objInfo[start], &objInfo[mid],
							&objInfo[end - 1] + 1,
							[dim](const BVHObjInfo &a,
								const BVHObjInfo &b) {
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
									objInfo[i].centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							Assert(b >= 0 && b < nBuckets);
							buckets[b].count++;
							buckets[b].bounds =
								Union(buckets[b].bounds, objInfo[i].bounds);
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

						// Either create leaf or split objs at selected SAH
						// bucket
						float leafCost = nObjs;
						if (nObjs > bvhAccel->maxObjsInNode || minCost < leafCost) {
							BVHObjInfo *pmid = std::partition(
								&objInfo[start], &objInfo[end - 1] + 1,
								[=](const BVHObjInfo &pi) {
								int b = nBuckets *
									centroidBounds.Offset(pi.centroid)[dim];
								if (b == nBuckets) b = nBuckets - 1;
								Assert(b >= 0 && b < nBuckets);
								return b <= minCostSplitBucket;
							});
							mid = pmid - &objInfo[0];
						}
						else {
							// Create leaf _BVHBuildNode_
							int firstPrimOffset = orderedObjs.size();
							for (int i = start; i < end; ++i) {
								int primNum = objInfo[i].objNumber;
								orderedObjs.push_back(bvhAccel->objs[primNum]);
							}
							InitLeaf(node, firstPrimOffset, nObjs, bounds);
							return node;
						}
					}
					break;
				}
				}
				InitInterior(node, dim,
					recursiveBuild(bvhAccel, objInfo, start, mid,
						totalNodes, orderedObjs),
					recursiveBuild(bvhAccel, objInfo, mid, end,
						totalNodes, orderedObjs));
			}
		}
		return node;
	}

	BVHBuildNode *BVHSystem::HLBVHBuild(
		ComponentHandle<BVHAccel> bvhAccel,
		const std::vector<BVHObjInfo> &objInfo,
		int *totalNodes,
		std::vector<ObjectID> &orderedObjs)  {
		return nullptr;
		
		// Compute bounding box of all obj centroids
		BBox bounds;
		for (const BVHObjInfo &pi : objInfo)
			bounds = Union(bounds, pi.centroid);

		// Compute Morton indices of objs
		std::vector<MortonObj> mortonObjs(objInfo.size());
		ParallelFor([&](int i) {
			// Initialize _mortonObjs[i]_ for _i_th obj
			constexpr int mortonBits = 10;
			constexpr int mortonScale = 1 << mortonBits;
			mortonObjs[i].objIndex = objInfo[i].objNumber;
			Vector3dF centroidOffset = bounds.Offset(objInfo[i].centroid);
			mortonObjs[i].mortonCode = EncodeMorton3(centroidOffset * mortonScale);
		}, objInfo.size(), 512);

		// Radix sort obj Morton indices
		RadixSort(&mortonObjs);

		// Create LBVH treelets at bottom of BVH

		// Find intervals of objs for each treelet
		std::vector<LBVHTreelet> treeletsToBuild;
		for (int start = 0, end = 1; end <= (int)mortonObjs.size(); ++end) {
			uint32_t mask = 0b00111111111111000000000000000000;
			if (end == (int)mortonObjs.size() ||
				((mortonObjs[start].mortonCode & mask) !=
				(mortonObjs[end].mortonCode & mask))) {
				// Add entry to _treeletsToBuild_ for this treelet
				int nObjs = end - start;
				int maxBVHNodes = 2 * nObjs;
				BVHBuildNode *nodes = GetPool<BVHBuildNode>()->newElement();
				//n = maxBVHNodes, runConstructor = false
				treeletsToBuild.push_back({ start, nObjs, nodes });
				start = end;
			}
		}

		// Create LBVHs for treelets in parallel
		std::atomic<int> atomicTotal(0), orderedObjsOffset(0);
		orderedObjs.resize(bvhAccel->objs.size());
		ParallelFor([&](int i) {
			// Generate _i_th LBVH treelet
			int nodesCreated = 0;
			const int firstBitIndex = 29 - 12;
			LBVHTreelet &tr = treeletsToBuild[i];
			tr.buildNodes = emitLBVH(bvhAccel, tr.buildNodes, objInfo, &mortonObjs[tr.startIndex],
				tr.nObjs, &nodesCreated, orderedObjs,
				&orderedObjsOffset, firstBitIndex);
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
	}

	BVHBuildNode *BVHSystem::emitLBVH(ComponentHandle<BVHAccel> bvhAccel,
		BVHBuildNode *&buildNodes,
		const std::vector<BVHObjInfo> &objInfo,
		MortonObj *mortonObjs, int nObjs, int *totalNodes,
		std::vector<ObjectID> &orderedObjs,
		std::atomic<int> *orderedObjsOffset, int bitIndex) {
		Assert(nObjs > 0);
		if (bitIndex == -1 || nObjs < bvhAccel->maxObjsInNode) {
			// Create and return leaf node of LBVH treelet
			(*totalNodes)++;
			BVHBuildNode *node = buildNodes++;
			BBox bounds;
			int firstPrimOffset = orderedObjsOffset->fetch_add(nObjs);
			for (int i = 0; i < nObjs; ++i) {
				int objIndex = mortonObjs[i].objIndex;
				orderedObjs[firstPrimOffset + i] = bvhAccel->objs[objIndex];
				bounds = Union(bounds, objInfo[objIndex].bounds);
			}
			InitLeaf(node, firstPrimOffset, nObjs, bounds);
			return node;
		}
		else {
			int mask = 1 << bitIndex;
			// Advance to next subtree level if there's no LBVH split for this bit
			if ((mortonObjs[0].mortonCode & mask) ==
				(mortonObjs[nObjs - 1].mortonCode & mask))
				return emitLBVH(bvhAccel, buildNodes, objInfo, mortonObjs, nObjs,
					totalNodes, orderedObjs, orderedObjsOffset,
					bitIndex - 1);

			// Find LBVH split point for this dimension
			int searchStart = 0, searchEnd = nObjs - 1;
			while (searchStart + 1 != searchEnd) {
				Assert(searchStart != searchEnd);
				int mid = (searchStart + searchEnd) / 2;
				if ((mortonObjs[searchStart].mortonCode & mask) ==
					(mortonObjs[mid].mortonCode & mask))
					searchStart = mid;
				else {
					Assert((mortonObjs[mid].mortonCode & mask) ==
						(mortonObjs[searchEnd].mortonCode & mask));
					searchEnd = mid;
				}
			}
			int splitOffset = searchEnd;
			Assert(splitOffset <= nObjs - 1);
			Assert((mortonObjs[splitOffset - 1].mortonCode & mask) !=
				(mortonObjs[splitOffset].mortonCode & mask));

			// Create and return interior LBVH node
			(*totalNodes)++;
			BVHBuildNode *node = buildNodes++;
			BVHBuildNode *lbvh[2] = {
				emitLBVH(bvhAccel, buildNodes, objInfo, mortonObjs, splitOffset,
				totalNodes, orderedObjs, orderedObjsOffset,
				bitIndex - 1),
				emitLBVH(bvhAccel, buildNodes, objInfo, &mortonObjs[splitOffset],
				nObjs - splitOffset, totalNodes, orderedObjs,
				orderedObjsOffset, bitIndex - 1) };
			Axis axis = static_cast<Axis>(bitIndex % 3);
			InitInterior(node, axis, lbvh[0], lbvh[1]);
			return node;
		}
	}

	BVHBuildNode *BVHSystem::buildUpperSAH(std::vector<BVHBuildNode *> &treeletRoots,
		int start, int end,
		int *totalNodes) {
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
		InitInterior(node,
			dim, buildUpperSAH(treeletRoots, start, mid, totalNodes),
			buildUpperSAH(treeletRoots, mid, end, totalNodes));
		return node;
	}

	int BVHSystem::flattenBVHTree(ComponentHandle<BVHAccel> bvhAccel, BVHBuildNode *node, int *offset) {
		LinearBVHNode *linearNode = &bvhAccel->nodes[*offset];
		linearNode->bounds = node->bounds;
		int myOffset = (*offset)++;
		if (node->nObjs > 0) {
			Assert(!node->children[0] && !node->children[1]);
			Assert(node->nObjs < 65536);
			linearNode->objsOffset = node->firstPrimOffset;
			linearNode->nObjs = node->nObjs;
		}
		else {
			// Create interior flattened BVH node
			linearNode->axis = node->splitAxis;
			linearNode->nObjs = 0;
			flattenBVHTree(bvhAccel, node->children[0], offset);
			linearNode->secondChildOffset =
				flattenBVHTree(bvhAccel, node->children[1], offset);
		}
		return myOffset;
	}


	void BVHSystem::optimize(ComponentHandle<BVHAccel> bvhAccel) {
		/*
		if (LEAF_OBJ_MAX != 1) {
			throw new Exception("In order to use optimize, you must set LEAF_OBJ_MAX=1");
		}

		while (refitNodes.Count > 0) {
			int maxdepth = refitNodes.Max(n = > n.depth);

			var sweepNodes = refitNodes.Where(n = > n.depth == maxdepth).ToList();
			sweepNodes.ForEach(n = > refitNodes.Remove(n));

			sweepNodes.ForEach(n = > n.tryRotate(this));
		}*/
	}

	void BVHSystem::CreateBVHAccel(ComponentHandle<BVHAccel> bvhAccel,
		const std::vector<ObjectID> &objs, std::string splitMethodName, int maxObjsInNode) {
		if (splitMethodName == "") {
			splitMethodName = "sah";
		}
		if (!maxObjsInNode) {
			maxObjsInNode = 4;
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
		CreateBVHAccel(bvhAccel, objs, maxObjsInNode, splitMethod);
	}

	void BVHSystem::InitLeaf(BVHBuildNode * node, int first, int n, const BBox &b) {
		node->firstPrimOffset = first;
		node->nObjs = n;
		node->bounds = b;
		node->children[0] = node->children[1] = nullptr;
	}
	void BVHSystem::InitInterior(BVHBuildNode * node, Axis axis, BVHBuildNode *c0, BVHBuildNode *c1) {
		node->children[0] = c0;
		node->children[1] = c1;
		node->bounds = Union(c0->bounds, c1->bounds);
		node->splitAxis = axis;
		node->nObjs = 0;
	}

};