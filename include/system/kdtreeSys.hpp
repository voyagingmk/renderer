#ifndef RENDERER_SYSTEM_KDTREE_HPP
#define RENDERER_SYSTEM_KDTREE_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/miscEvent.hpp"
#include "com/kdtree.hpp"

using namespace ecs;
using json = nlohmann::json;

namespace renderer {
	class KdTreeSystem : public System<KdTreeSystem>, public Receiver<KdTreeSystem> {
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const DebugDrawKdTreeEvent &evt);

		void receive(const CreateKdTreeEvent& evt);

	private:
		void CreateKdTreeAccel(ComponentHandle<KdTreeAccel>, const std::vector<ObjectID> &p,
			int isectCost = 80, int traversalCost = 1,
			float emptyBonus = 0.5, int maxObjs = 1, int maxDepth = -1);

		void BuildTree(ComponentHandle<KdTreeAccel> bvhAccel, int nodeNum, const BBox &bounds,
			const std::vector<BBox> &primBounds, int *primNums,
			int nobjs, int depth,
			const std::unique_ptr<BoundEdge[]> edges[3], int *objs0,
			int *objs1, int badRefines = 0);

		void InitLeaf(KdAccelNode* node, int *primNums, int np,
			std::vector<int> *objIndices);

		void CreateKdTreeAccel(ComponentHandle<KdTreeAccel>,
			const std::vector<ObjectID> &objs, const json &config);

	};


};
#endif  // PBRT_ACCELERATORS_KDTREEACCEL_H
