#ifndef RENDERER_BUFFERMGR_HPP
#define RENDERER_BUFFERMGR_HPP

#include "base.hpp"
#include "matrix.hpp"
#include "transform.hpp"

namespace renderer {
    
    typedef uint32_t bufferID;
    
    struct BufferSet {
        size_t triangles;
        bufferID vao;
        bufferID vbo;
        bufferID ebo;
    };
    
    typedef std::map<const char *, BufferSet> BufferDict;
    
    class BufferMgrBase {
    protected:
        BufferDict bufferDict;
    public:
        virtual	~BufferMgrBase();
        BufferSet GetBufferSet(const char * aliasname) {
            return bufferDict[aliasname];
        }
        virtual BufferSet CreateBuffer(const char* aliasname, std::vector<float>& vertexes, std::vector<float>& texcoords, std::vector<unsigned int>& indices) {
            return BufferSet();
        }
        virtual void DrawBuffer(const char* aliasname) {}
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
        virtual BufferSet CreateBuffer(const char* aliasname, std::vector<float>& vertices, std::vector<float>& texcoords, std::vector<unsigned int>& indices);
        virtual void DrawBuffer(const char* aliasname);
    };
#endif
    
}

#endif
