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
        virtual BufferSet CreateBuffer(const std::string& aliasname, Mesh& mesh) {
            return BufferSet();
        }
        virtual void DrawBuffer(const std::string& aliasname) {}
        virtual void release() {}
        virtual FrameBuf createFrameBuffer(size_t width, size_t height, BufType depthType) { return FrameBuf(); }
        virtual void DestroyFrameBuffer(FrameBuf& buf) {}
        virtual void UseFrameBuffer(FrameBuf& buf) {}
        virtual void UnuseFrameBuffer() {}
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
        virtual BufferSet CreateBuffer(const std::string& aliasname, Mesh& mesh);
        virtual void DrawBuffer(const std::string& aliasname);
        virtual FrameBuf createFrameBuffer(size_t width, size_t height, BufType depthType);
        virtual void DestroyFrameBuffer(FrameBuf& buf);
        virtual void UseFrameBuffer(FrameBuf& buf);
        virtual void UnuseFrameBuffer();
    };
#endif
    
}

#endif
