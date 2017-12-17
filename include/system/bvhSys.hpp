#ifndef RENDERER_SYSTEM_BVH_HPP
#define RENDERER_SYSTEM_BVH_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/bvh.hpp"
#include "event/miscEvent.hpp"


using namespace ecs;

namespace renderer {

	class BVHSystem : public System<BVHSystem>, public Receiver<BVHSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const DebugDrawBVHEvent &evt);

		void receive(const CreateBVHEvent& evt);

	private:

		void CreateBVHAccel(ComponentHandle<BVHAccel>,
			const std::vector<ObjectID> &prims, std::string splitMethodName, int maxPrimsInNode);

		void CreateBVHAccel(ComponentHandle<BVHAccel>, 
			const std::vector<ObjectID> &p,
			int maxPrimsInNode = 1,
			BVHAccel::SplitMethod splitMethod = BVHAccel::SplitMethod::SAH);

		BVHBuildNode *recursiveBuild(ComponentHandle<BVHAccel> bvhAccel, std::vector<BVHPrimitiveInfo> &primitiveInfo,
			int start, int end, int *totalNodes,
			std::vector<ObjectID> &orderedPrims);

		BVHBuildNode *HLBVHBuild(const std::vector<BVHPrimitiveInfo> &primitiveInfo,
			int *totalNodes,
			std::vector<ObjectID> &orderedPrims) const;

		BVHBuildNode *emitLBVH(ComponentHandle<BVHAccel> bvhAccel,
			BVHBuildNode *&buildNodes,
			const std::vector<BVHPrimitiveInfo> &primitiveInfo,
			MortonPrimitive *mortonPrims, int nPrimitives, int *totalNodes,
			std::vector<ObjectID> &orderedPrims,
			std::atomic<int> *orderedPrimsOffset, int bitIndex) const;

		BVHBuildNode *buildUpperSAH(std::vector<BVHBuildNode *> &treeletRoots,
			int start, int end, int *totalNodes) const;

		int flattenBVHTree(ComponentHandle<BVHAccel> bvhAccel, BVHBuildNode *node, int *offset);
	};
};



#endif

