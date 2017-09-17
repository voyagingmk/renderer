#ifndef RENDERER_SYSTEM_BUFFER_HPP
#define RENDERER_SYSTEM_BUFFER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/meshes.hpp"
#include "com/bufferCom.hpp"
#include "event/bufferEvent.hpp"



using namespace ecs;

namespace renderer {

	class BufferSystem : public System<BufferSystem>, public Receiver<BufferSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const ComponentAddedEvent<Meshes> &evt);
		
		void receive(const DrawBufferEvent& evt);

	private:

		MeshBufferRef createMeshBuffer(const OneMesh& mesh);
	};
};


#endif
