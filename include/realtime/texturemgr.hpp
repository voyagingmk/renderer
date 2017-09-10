#ifndef RENDERER_TEXTUREMGR_HPP
#define RENDERER_TEXTUREMGR_HPP

#include "base.hpp"
#include "com/glcommon.hpp"

namespace renderer
{


class TextureMgrBase
{
  protected:
    TextureDict texDict;
    std::string dirpath;

  protected:
    virtual ~TextureMgrBase();

  public:
    void setTextureDirPath(const char *path);
    virtual TexRef loadTexture(const char *filename, std::string &aliasname, bool hasAlpha = false, bool toLinear = true)
    {
        return TexRef();
    }
    virtual TexRef loadCubeMap(std::string filename[6], std::string &&aliasname)
    {
        return TexRef();
    }
    virtual void destroyTexture(std::string &&aliasname) {}
    TexRef getTexRef(std::string &&aliasname);
    void release();
};

#ifdef USE_GL

class TextureMgrOpenGL : public TextureMgrBase
{
  private:
    TextureMgrOpenGL() {}

  public:
    static TextureMgrOpenGL &getInstance()
    {
        static TextureMgrOpenGL mgr;
        return mgr;
    }
    virtual TexRef loadTexture(const char *filename, std::string &aliasname, bool hasAlpha = false, bool toLinear = true);
    virtual TexRef CreateDepthTexture(DepthTexType dtType, size_t width, size_t height);
    virtual TexRef loadCubeMap(std::string filename[6], std::string &&aliasname);
    void activateTexture(uint32_t idx, TexRef texRef);
    void DisableTexture(uint32_t idx);
    void activateTexture(uint32_t idx, std::string &&aliasname);
    virtual void destroyTexture(std::string &&aliasname);
};

typedef TextureMgrOpenGL TextureMgr;
#endif
};

#endif
