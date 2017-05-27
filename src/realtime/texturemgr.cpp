#include "stdafx.h"
#include "texturemgr.hpp"
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

texID TextureMgrBase::getTexID(const char* aliasname) {
    return texDict[aliasname];
}



#ifdef USE_GL

texID TextureMgrOpenGL::loadTexture(const char* filename, const char* aliasname)
{
    // Load and create a texture
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char* image = SOIL_load_image((dirpath + std::string(filename)).c_str(), &width, &height, 0, SOIL_LOAD_RGB);
    std::cout<< "SOIL: image[" << filename << "] loaded, w:" << width << ", h:" << height << std::endl;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
    texDict.insert({aliasname, texID});
    return texID;
}

void TextureMgrOpenGL::destroyTexture(texID texID) {
    
}


#endif
