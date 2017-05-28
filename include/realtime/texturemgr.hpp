#ifndef RENDERER_TEXTUREMGR_HPP
#define RENDERER_TEXTUREMGR_HPP

#include "base.hpp"

namespace renderer {
    
#ifdef USE_GL
    typedef GLuint texID;
#else 
    typedef unsigned int texID;
#endif
    
    typedef std::map<const char *, texID> TextureDict;
    
	class TextureMgrBase {
	protected:
        TextureDict texDict;
        std::string dirpath;
	protected:
        virtual	~TextureMgrBase();
    public:
        void setTextureDirPath(const char* path);
        virtual texID loadTexture(const char* filename, const char* aliasname) { }
        virtual void destroyTexture(texID texID) {}
         texID getTexID(const char* aliasname);
		void release();
	};


#ifdef USE_GL
    
	class TextureMgrOpenGL: public TextureMgrBase {
	private:
		TextureMgrOpenGL() {}
	public:
		static TextureMgrOpenGL& getInstance() {
			static TextureMgrOpenGL mgr;
			return mgr;
		}
        virtual texID loadTexture(const char* filename, const char* aliasname);
        void activateTexture(uint32_t idx, texID texID);
        virtual void destroyTexture(texID texID);
	};

#endif

	// using ShaderMgr = ShaderMgrOpenGL;
};

#endif
