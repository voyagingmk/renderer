#ifndef RENDERER_TEXTUREMGR_HPP
#define RENDERER_TEXTUREMGR_HPP

#include "base.hpp"
#include "glcommon.hpp"

namespace renderer {

    typedef std::map<const char *, TexRef> TextureDict;
    
	class TextureMgrBase {
	protected:
        TextureDict texDict;
        std::string dirpath;
	protected:
        virtual	~TextureMgrBase();
    public:
        void setTextureDirPath(const char* path);
        virtual TexRef loadTexture(const char* filename, const char* aliasname, bool hasAlpha = false, bool toLinear = true) {
            return TexRef();
        }
        virtual TexRef loadCubeMap(std::string filename[6], const char* aliasname) {
            return TexRef();
        }
        virtual void destroyTexture(const char* aliasname) {}
        TexRef getTexRef(const char* aliasname);
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
        virtual TexRef loadTexture(const char* filename, const char* aliasname, bool hasAlpha = false, bool toLinear = true);
        virtual TexRef CreateDepthTexture(DepthTexType dtType,size_t width, size_t height);
        virtual TexRef loadCubeMap(std::string filename[6], const char* aliasname);
        void activateTexture(uint32_t idx, TexRef texRef);
        void DisableTexture(uint32_t idx);
        void activateTexture(uint32_t idx, const char* aliasname);
        virtual void destroyTexture(const char* aliasname);
	};

#endif

	// using ShaderMgr = ShaderMgrOpenGL;
};

#endif
