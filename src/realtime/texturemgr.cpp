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

TexRef TextureMgrBase::getTexRef(std::string&& aliasname) {
    return texDict[aliasname];
}



#ifdef USE_GL

TexRef TextureMgrOpenGL::loadTexture(const char* filename, std::string& aliasname, bool hasAlpha, bool toLinear)
{
    // Load and create a texture
    TexRef texRef;
    glGenTextures(1, &texRef.texID);
    glBindTexture(GL_TEXTURE_2D, texRef.texID); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
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
    
    auto format = GL_RGB;
    auto formatWithAlpha = GL_RGBA;
    if (toLinear) {
        format = GL_SRGB;
        formatWithAlpha = GL_SRGB_ALPHA;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha? formatWithAlpha: format, width, height, 0, hasAlpha? GL_RGBA: GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
    texRef.width = width;
    texRef.height = height;
    texRef.type = TexType::Tex2D;
    texDict.insert({aliasname, texRef});
    return texRef;
}


TexRef TextureMgrOpenGL::CreateDepthTexture(DepthTexType dtType, size_t width, size_t height) {
    TexRef texRef;
    texRef.width = width;
    texRef.height = height;
    glGenTextures(1, &texRef.texID);
    if(dtType == DepthTexType::DepthOnly ||
       dtType == DepthTexType::DepthStencil) {
        texRef.type = TexType::Tex2D;
        glBindTexture(GL_TEXTURE_2D, texRef.texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        if(dtType == DepthTexType::DepthStencil)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,width, height, 0, GL_DEPTH_COMPONENT,  GL_FLOAT, 0);
        if(dtType == DepthTexType::DepthStencil)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        texRef.type = TexType::CubeMap;
        glBindTexture(GL_TEXTURE_CUBE_MAP, texRef.texID);
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    return texRef;
}

TexRef TextureMgrOpenGL::loadCubeMap(std::string filename[6], std::string&& aliasname) {
    TexRef texRef;
    glGenTextures(1, &texRef.texID);
    glActiveTexture(GL_TEXTURE0);
    
    int width, height;
    unsigned char* image;
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, texRef.texID);
    for (uint32_t i = 0; i < 6; i++)
    {
        image = SOIL_load_image((dirpath + filename[i]).c_str(), &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    texDict.insert({aliasname, texRef});
    return texRef;
}

void TextureMgrOpenGL::activateTexture(uint32_t idx, TexRef texRef) {
    glActiveTexture(GL_TEXTURE0 + idx);
    if(texRef.type == TexType::Tex2D) {
        glBindTexture(GL_TEXTURE_2D, texRef.texID);
    } else if(texRef.type == TexType::CubeMap) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, texRef.texID);
    }
}

void TextureMgrOpenGL::DisableTexture(uint32_t idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureMgrOpenGL::activateTexture(uint32_t idx, std::string&& aliasname) {
    auto it = texDict.find(aliasname);
    if(it == texDict.end()) {
        return;
    }
    activateTexture(idx, it->second);
}

void TextureMgrOpenGL::destroyTexture(std::string&& aliasname) {
    auto it = texDict.find(aliasname);
    if(it == texDict.end()) {
        return;
    }
    glDeleteTextures(1, &it->second.texID);
    texDict.erase(it);
}


#endif
