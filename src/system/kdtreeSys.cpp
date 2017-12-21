#include "stdafx.h"
#include "system/kdtreeSys.hpp"
#include "com/geometry.hpp"
#include "com/spatialData.hpp"
#include "com/shader.hpp"
#include "event/shaderEvent.hpp"
#include "event/bufferEvent.hpp"
#include "com/miscCom.hpp"
#include "utils/memory.hpp"

using namespace std;


namespace renderer {

	void KdTreeSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("KdTreeSystem init\n");
		evtMgr.on<CreateKdTreeEvent>(*this);
		evtMgr.on<DebugDrawKdTreeEvent>(*this);
	}

	void KdTreeSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}
    
	void KdTreeSystem::receive(const DebugDrawKdTreeEvent &evt) {

	}

	void KdTreeSystem::receive(const CreateKdTreeEvent &evt) {
		Object objScene = evt.objScene;
		Object objKdTree = evt.objKdTree;
		auto bvhAccel = objKdTree.addComponent<KdTreeAccel>();

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

		CreateKdTreeAccel(bvhAccel, objs, {});
	}

	// KdTreeAccel Method Definitions
	void KdTreeSystem::CreateKdTreeAccel(ComponentHandle<KdTreeAccel> kdTreeAccel, const std::vector<ObjectID> &p,
		int isectCost, int traversalCost, float emptyBonus,
		int maxObjs, int maxDepth) {
		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();

		kdTreeAccel->isectCost = isectCost;
		kdTreeAccel->traversalCost = traversalCost;
		kdTreeAccel->maxObjs = maxObjs;
		kdTreeAccel->emptyBonus = emptyBonus;
		kdTreeAccel->objs = p;
		// Build kd-tree for accelerator
		kdTreeAccel->nextFreeNode = kdTreeAccel->nAllocedNodes = 0;
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * Log2Int(kdTreeAccel->objs.size()));

		// Compute bounds for kd-tree construction
		std::vector<BBox> primBounds;
		primBounds.reserve(kdTreeAccel->objs.size());
		for (const ObjectID &objID : kdTreeAccel->objs) {
			Object obj = m_objMgr->get(objID);
			auto meshRef = obj.component<MeshRef>();
			auto spatialData = obj.component<SpatialData>();
			Mesh& mesh = meshSet->getMesh(meshRef->meshID);
			auto bound = mesh.Bound();
			auto worldBound = (spatialData->o2w)(bound);
			kdTreeAccel->bounds = Union(kdTreeAccel->bounds, worldBound);
			primBounds.push_back(worldBound);
		}

		// Allocate working memory for kd-tree construction
		std::unique_ptr<BoundEdge[]> edges[3];
		for (int i = 0; i < 3; ++i)
			edges[i].reset(new BoundEdge[2 * kdTreeAccel->objs.size()]);
		std::unique_ptr<int[]> objs0(new int[kdTreeAccel->objs.size()]);
		std::unique_ptr<int[]> objs1(new int[(maxDepth + 1) * kdTreeAccel->objs.size()]);

		// Initialize _primNums_ for kd-tree construction
		std::unique_ptr<int[]> primNums(new int[kdTreeAccel->objs.size()]);
		for (size_t i = 0; i < kdTreeAccel->objs.size(); ++i) primNums[i] = i;

		// Start recursive construction of kd-tree
		BuildTree(kdTreeAccel, 0, kdTreeAccel->bounds, primBounds, primNums.get(), kdTreeAccel->objs.size(),
			maxDepth, edges, objs0.get(), objs1.get());
	}

	void KdTreeSystem::InitLeaf(KdAccelNode* node, int *primNums, int np,
		std::vector<int> *objIndices) {
		node->flags = 3;
		node->nObjs |= (np << 2);
		// Store obj ids for leaf node
		if (np == 0)
			node->oneObj = 0;
		else if (np == 1)
			node->oneObj = primNums[0];
		else {
			node->objIndicesOffset = objIndices->size();
			for (int i = 0; i < np; ++i) objIndices->push_back(primNums[i]);
		}
	}


	void KdTreeSystem::BuildTree(ComponentHandle<KdTreeAccel> bvhAccel, int nodeNum, const BBox &nodeBounds,
		const std::vector<BBox> &allPrimBounds,
		int *primNums, int nObjs, int depth,
		const std::unique_ptr<BoundEdge[]> edges[3],
		int *objs0, int *objs1, int badRefines) {
		assert(nodeNum == bvhAccel->nextFreeNode);
		// Get next free node from _nodes_ array
		if (bvhAccel->nextFreeNode == bvhAccel->nAllocedNodes) {
			int nNewAllocNodes = std::max(2 * bvhAccel->nAllocedNodes, 512);
			KdAccelNode *n = AllocAligned<KdAccelNode>(nNewAllocNodes);
			if (bvhAccel->nAllocedNodes > 0) {
				memcpy(n, bvhAccel->nodes, bvhAccel->nAllocedNodes * sizeof(KdAccelNode));
				FreeAligned(bvhAccel->nodes);
			}
			bvhAccel->nodes = n;
			bvhAccel->nAllocedNodes = nNewAllocNodes;
		}
		++bvhAccel->nextFreeNode;

		// Initialize leaf node if termination criteria met
		if (nObjs <= bvhAccel->maxObjs || depth == 0) {
			InitLeaf(&bvhAccel->nodes[nodeNum], primNums, nObjs, &bvhAccel->objIndices);
			return;
		}

		// Initialize interior node and continue recursion

		// Choose split axis position for interior node
		int bestAxis = -1, bestOffset = -1;
		float bestCost = Infinity;
		float oldCost = bvhAccel->isectCost * float(nObjs);
		float totalSA = nodeBounds.SurfaceArea();
		float invTotalSA = 1 / totalSA;
		Vector3dF d = nodeBounds.pMax - nodeBounds.pMin;

		// Choose which axis to split along
		int axis = static_cast<int>(nodeBounds.MaximumExtent());
		int retries = 0;
	retrySplit:

		// Initialize edges for _axis_
		for (int i = 0; i < nObjs; ++i) {
			int pn = primNums[i];
			const BBox &bounds = allPrimBounds[pn];
			edges[axis][2 * i] = BoundEdge(bounds.pMin[axis], pn, true);
			edges[axis][2 * i + 1] = BoundEdge(bounds.pMax[axis], pn, false);
		}

		// Sort _edges_ for _axis_
		std::sort(&edges[axis][0], &edges[axis][2 * nObjs],
			[](const BoundEdge &e0, const BoundEdge &e1) -> bool {
			if (e0.t == e1.t)
				return (int)e0.type < (int)e1.type;
			else
				return e0.t < e1.t;
		});

		// Compute cost of all splits for _axis_ to find best
		int nBelow = 0, nAbove = nObjs;
		for (int i = 0; i < 2 * nObjs; ++i) {
			if (edges[axis][i].type == EdgeType::End) --nAbove;
			float edgeT = edges[axis][i].t;
			if (edgeT > nodeBounds.pMin[axis] && edgeT < nodeBounds.pMax[axis]) {
				// Compute cost for split at _i_th edge

				// Compute child surface areas for split at _edgeT_
				int otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;
				float belowSA = 2 * (d[otherAxis0] * d[otherAxis1] +
					(edgeT - nodeBounds.pMin[axis]) *
					(d[otherAxis0] + d[otherAxis1]));
				float aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] +
					(nodeBounds.pMax[axis] - edgeT) *
					(d[otherAxis0] + d[otherAxis1]));
				float pBelow = belowSA * invTotalSA;
				float pAbove = aboveSA * invTotalSA;
				float eb = (nAbove == 0 || nBelow == 0) ? bvhAccel->emptyBonus : 0;
				float cost =
					bvhAccel->traversalCost +
					bvhAccel->isectCost * (1 - eb) * (pBelow * nBelow + pAbove * nAbove);

				// Update best split if this is lowest cost so far
				if (cost < bestCost) {
					bestCost = cost;
					bestAxis = axis;
					bestOffset = i;
				}
			}
			if (edges[axis][i].type == EdgeType::Start) ++nBelow;
		}
		assert(nBelow == nObjs && nAbove == 0);

		// Create leaf if no good splits were found
		if (bestAxis == -1 && retries < 2) {
			++retries;
			axis = (axis + 1) % 3;
			goto retrySplit;
		}
		if (bestCost > oldCost) ++badRefines;
		if ((bestCost > 4 * oldCost && nObjs < 16) || bestAxis == -1 ||
			badRefines == 3) {
			InitLeaf(&bvhAccel->nodes[nodeNum], primNums, nObjs, &bvhAccel->objIndices);
			return;
		}

		// Classify objs with respect to split
		int n0 = 0, n1 = 0;
		for (int i = 0; i < bestOffset; ++i)
			if (edges[bestAxis][i].type == EdgeType::Start)
				objs0[n0++] = edges[bestAxis][i].primNum;
		for (int i = bestOffset + 1; i < 2 * nObjs; ++i)
			if (edges[bestAxis][i].type == EdgeType::End)
				objs1[n1++] = edges[bestAxis][i].primNum;

		// Recursively initialize children nodes
		float tSplit = edges[bestAxis][bestOffset].t;
		BBox bounds0 = nodeBounds, bounds1 = nodeBounds;
		bounds0.pMax[bestAxis] = bounds1.pMin[bestAxis] = tSplit;
		BuildTree(bvhAccel, nodeNum + 1, bounds0, allPrimBounds, objs0, n0, depth - 1, edges,
			objs0, objs1 + nObjs, badRefines);
		int aboveChild = bvhAccel->nextFreeNode;
		bvhAccel->nodes[nodeNum].InitInterior(bestAxis, aboveChild, tSplit);
		BuildTree(bvhAccel, aboveChild, bounds1, allPrimBounds, objs1, n1, depth - 1, edges,
			objs0, objs1 + nObjs, badRefines);
	}


	void KdTreeSystem::CreateKdTreeAccel(
		ComponentHandle<KdTreeAccel> bvhAccel,
		const std::vector<ObjectID> &objs, const json &config) {
		int isectCost = config["intersectcost"] || 80;
		int travCost = config["traversalcost"] || 1;
		float emptyBonus = config["emptybonus"] || 0.5f;
		int maxObjs = config["maxobjs"] || 1;
		int maxDepth = config["maxdepth"] || -1;
		return CreateKdTreeAccel(bvhAccel, objs, isectCost, travCost, emptyBonus,
			maxObjs, maxDepth);
	}

};
