#ifndef RENDERER_SYSTEM_BUFFER_HPP
#define RENDERER_SYSTEM_BUFFER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/mesh.hpp"
#include "com/bufferCom.hpp"
#include "event/bufferEvent.hpp"



using namespace ecs;

namespace renderer {

	class BufferSystem : public System<BufferSystem>, public Receiver<BufferSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr);

		void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;

		void receive(const CreateMeshBufferEvent &evt);

		void receive(const CreateSkyboxBufferEvent &evt);

		void receive(const DrawMeshBufferEvent& evt);

		void receive(const EnabledMeshBufferInstanceEvent& evt);

		void receive(const DrawOneMeshBufferEvent& evt);

		void receive(const AddColorBufferEvent& evt);
        
        void receive(const CreateDpethBufferEvent& evt);

		void receive(const CreateColorBufferEvent& evt);

		void receive(const DestroyColorBufferEvent& evt);

		void receive(const UseColorBufferEvent& evt);

		void receive(const UnuseColorBufferEvent& evt);

		void receive(const CreateGBufferEvent& evt);
		
		void receive(const DestroyGBufferEvent& evt);

		void receive(const UseGBufferEvent& evt);

		void receive(const UnuseGBufferEvent& evt);

		void receive(const CopyGBufferDepth2ColorBufferEvent& evt);

		void receive(const CreateInstanceBufferEvent& evt);

	private:
		void drawMeshBuffer(const MeshBufferRef& meshBuffer);

		void DestroyFrameBuffer(FrameBufferBase& buf);

		void UseFrameBuffer(FrameBufferBase& buf);

		void UnuseFrameBuffer(FrameBufferBase& buf);

        ColorBufferRef CreateDepthFrameBuffer(DepthTexType dtType, std::string texAliasname, size_t width);
            
		void CreateInstanceBuffer(MeshBufferRef& buf, size_t insNum, void* data);

		MeshBufferRef CreateMeshBuffer(const SubMesh& mesh);

		void EnabledMeshBufferInstance(MeshBufferRef& buf, const InstanceBufferRef& insBuf);

		MeshBufferRef CreateSkyboxBuffer();
		
		ColorBufferRef CreateColorBuffer(
            size_t width, size_t height,
            int innerFormat,
            int format,
            int dataType = GL_UNSIGNED_BYTE,
            BufType depthType = BufType::None,
            size_t MSAA = 0,
            int texParam = GL_LINEAR);
		
		void DestroyColorBuffer(ColorBufferRef buf);

		GBufferRef CreateGBuffer(size_t width, size_t height);

		void DestroyGBuffer(GBufferRef);

		void CopyFrameBufferDepth(FrameBufferBase& buf, FrameBufferBase& buf2);
	};
};


#endif
