#ifndef RENDERER_SYSTEM_BUFFER_HPP
#define RENDERER_SYSTEM_BUFFER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/meshes.hpp"
#include "com/bufferCom.hpp"



using namespace ecs;

namespace renderer {

	class BufferSystem : public System<BufferSystem>, public Receiver<BufferSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
	
	private:

		void createMeshBuffer(OneMesh& mesh);
	};
};


#endif
