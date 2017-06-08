#ifndef RENDERER_TEXTUREMGR_HPP
#define RENDERER_TEXTUREMGR_HPP

#include "base.hpp"
#include "glcommon.hpp"

namespace renderer {

    typedef std::map<const char *, TexID> TextureDict;
    
	class TextureMgrBase {
	protected:
        TextureDict texDict;
        std::string dirpath;
	protected:
        virtual	~TextureMgrBase();
    public:
        void setTextureDirPath(const char* path);
        virtual TexID loadTexture(const char* filename, const char* aliasname, bool hasAlpha = false) { }
        virtual void destroyTexture(TexID TexID) {}
         TexID getTexID(const char* aliasname);
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
        virtual TexID loadTexture(const char* filename, const char* aliasname, bool hasAlpha = false);
        void activateTexture(uint32_t idx, TexID TexID);
        virtual void destroyTexture(TexID TexID);
	};

#endif

	// using ShaderMgr = ShaderMgrOpenGL;
};

#endif
