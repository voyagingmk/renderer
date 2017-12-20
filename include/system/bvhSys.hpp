#ifndef RENDERER_SYSTEM_BVH_HPP
#define RENDERER_SYSTEM_BVH_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/bvh.hpp"
#include "event/miscEvent.hpp"
#include <atomic>


using namespace ecs;

namespace renderer {

	class BVHSystem : public System<BVHSystem>, public Receiver<BVHSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const DebugDrawBVHEvent &evt);

		void receive(const CreateBVHEvent& evt);

		void receive(const ComponentRemovedEvent<BVHAccel> &evt);

	private:

		void CreateBVHAccel(ComponentHandle<BVHAccel>,
			const std::vector<ObjectID> &objs, std::string splitMethodName, int maxObjsInNode);

		void CreateBVHAccel(ComponentHandle<BVHAccel>, 
			const std::vector<ObjectID> &p,
			int maxObjsInNode = 1,
			BVHAccel::SplitMethod splitMethod = BVHAccel::SplitMethod::SAH);

		BVHBuildNode *recursiveBuild(ComponentHandle<BVHAccel> bvhAccel, std::vector<BVHObjInfo> &objInfo,
			int start, int end, int *totalNodes,
			std::vector<ObjectID> &orderedObjs);

		BVHBuildNode *HLBVHBuild(ComponentHandle<BVHAccel> bvhAccel, const std::vector<BVHObjInfo> &objInfo,
			int *totalNodes,
			std::vector<ObjectID> &orderedObjs);

		BVHBuildNode *emitLBVH(ComponentHandle<BVHAccel> bvhAccel,
			BVHBuildNode *&buildNodes,
			const std::vector<BVHObjInfo> &objInfo,
			MortonObj *mortonObjs, int nObjs, int *totalNodes,
			std::vector<ObjectID> &orderedObjs,
			std::atomic<int> *orderedObjsOffset, int bitIndex);

		BVHBuildNode *buildUpperSAH(std::vector<BVHBuildNode *> &treeletRoots,
			int start, int end, int *totalNodes);

		void optimize(ComponentHandle<BVHAccel> bvhAccel);

		int flattenBVHTree(ComponentHandle<BVHAccel> bvhAccel, BVHBuildNode *node, int *offset);

		void InitLeaf(BVHBuildNode * node, int first, int n, const BBox &b);
		
		void InitInterior(BVHBuildNode * node, Axis axis, BVHBuildNode *c0, BVHBuildNode *c1);
	};
};



#endif

