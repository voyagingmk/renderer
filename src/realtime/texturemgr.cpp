#include "stdafx.h"
#include "texturemgr.hpp"
#include "common.hpp"
#include "glutils.hpp"
#include "SOIL.h"

using namespace renderer;

TextureMgrBase::~TextureMgrBase() {
	release();
}

void TextureMgrBase::setTextureDirPath(const char* path) {
    dirpath = std::string(path);
}

void TextureMgrBase::release() {
	
}

TexID TextureMgrBase::getTexID(const char* aliasname) {
    return texDict[aliasname];
}



#ifdef USE_GL

TexID TextureMgrOpenGL::loadTexture(const char* filename, const char* aliasname, bool hasAlpha)
{
    // Load and create a texture
    GLuint TexID;
    glGenTextures(1, &TexID);
    glBindTexture(GL_TEXTURE_2D, TexID); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, hasAlpha? GL_CLAMP_TO_EDGE : GL_REPEAT);	// Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, hasAlpha? GL_CLAMP_TO_EDGE : GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char* image = SOIL_load_image((dirpath + std::string(filename)).c_str(), &width, &height, 0, hasAlpha? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
    
    std::cout<< "SOIL: image[" << filename << "] loaded, w:" << width << ", h:" << height << std::endl;
    
    glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha? GL_RGBA: GL_RGB, width, height, 0, hasAlpha? GL_RGBA: GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
    texDict.insert({aliasname, TexID});
    return TexID;
}

void TextureMgrOpenGL::activateTexture(uint32_t idx, TexID TexID) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, TexID);
}

void TextureMgrOpenGL::destroyTexture(TexID TexID) {
    
}


#endif
