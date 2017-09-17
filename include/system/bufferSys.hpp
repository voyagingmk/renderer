#ifndef RENDERER_SYSTEM_BUFFER_HPP
#define RENDERER_SYSTEM_BUFFER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/meshes.hpp"
#include "com/bufferCom.hpp"
#include "event/bufferEvent.hpp"
#include "com/glcommon.hpp"



using namespace ecs;

namespace renderer {

	class BufferSystem : public System<BufferSystem>, public Receiver<BufferSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const ComponentAddedEvent<Meshes> &evt);
		
		void receive(const DrawMeshBufferEvent& evt);
		void receive(const DrawMeshBufferEvent& evt);

	private:

		void DestroyFrameBuffer(FrameBufferBase& buf);

		void UseFrameBuffer(FrameBufferBase& buf);

		void UnuseFrameBuffer(FrameBufferBase& buf);

		MeshBufferRef CreateMeshBuffer(const OneMesh& mesh);
		
		ColorBufferRef CreateColorBuffer(size_t width, size_t height, BufType depthType, size_t MSAA);
		
		GBufferRef CreateGBuffer(size_t width, size_t height);
	};
};


#endif
