#ifndef RENDERER_BUFFERMGR_HPP
#define RENDERER_BUFFERMGR_HPP

#include "base.hpp"
#include "matrix.hpp"
#include "transform.hpp"
#include "mesh.hpp"

namespace renderer {
    
    typedef uint32_t bufferID;
    
    struct BufferSet {
        size_t triangles;
        bufferID vao;
        bufferID vbo;
        bufferID ebo;
    };
    
    typedef std::map<std::string, BufferSet> BufferDict;
    
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
    };
#endif
    
}

#endif
