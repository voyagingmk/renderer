#ifndef RENDERER_BUFFERMGR_HPP
#define RENDERER_BUFFERMGR_HPP

#include "base.hpp"
#include "glcommon.hpp"
#include "matrix.hpp"
#include "transform.hpp"
#include "mesh.hpp"

namespace renderer {

    class BufferMgrBase {
    protected:
        BufferDict bufferDict;
    public:
        virtual	~BufferMgrBase();
        BufferSet GetBufferSet(const char * aliasname) {
            return bufferDict[aliasname];
        }
        virtual BufferSet CreateMeshBuffer(const std::string& aliasname, Mesh& mesh) {
            return BufferSet();
        }
        virtual void DrawBuffer(const std::string& aliasname) {}
        virtual void release() {}
        virtual FrameBuf CreateDepthFrameBuffer(DepthTexType dtType, TexRef texRef) { return FrameBuf(); }
        virtual FrameBuf CreateColorFrameBuffer(size_t width, size_t height, BufType depthType, size_t MSAA = 0) { return FrameBuf(); }
        virtual void DestroyFrameBuffer(FrameBuf& buf) {}
        virtual void UseFrameBuffer(FrameBuf& buf) {}
        virtual void UnuseFrameBuffer(FrameBuf& buf) {}
    };

#ifdef USE_GL
    class BufferMgrOpenGL: public BufferMgrBase {
        
    private:
        BufferMgrOpenGL() {}
    public:
        static BufferMgrOpenGL& getInstance() {
            static BufferMgrOpenGL mgr;
            return mgr;
        }
        // override
        virtual BufferSet CreateMeshBuffer(const std::string& aliasname, Mesh& mesh);
        virtual void DrawBuffer(const std::string& aliasname);
        virtual FrameBuf CreateDepthFrameBuffer(DepthTexType dtType, TexRef texRef);
        virtual FrameBuf CreateColorFrameBuffer(size_t width, size_t height, BufType depthType, size_t MSAA = 0);
		virtual FrameBuf CreateGBuffer(size_t width, size_t height);
        virtual void DestroyFrameBuffer(FrameBuf& buf);
        virtual void UseFrameBuffer(FrameBuf& buf);
        virtual void UnuseFrameBuffer(FrameBuf& buf);
    };

	typedef BufferMgrOpenGL BufferMgr;
#endif
    
}

#endif
