#include "stdafx.h"
#include "system/textureSys.hpp"
#include "SOIL.h"

using namespace std;

namespace renderer {
	void TextureSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("TextureSystem init\n");
        evtMgr.on<LoadTextureEvent>(*this);
		evtMgr.on<DestroyTextureEvent>(*this); 
		evtMgr.on<ActiveTextureByIDEvent>(*this);
		evtMgr.on<ActiveTextureEvent>(*this);
		evtMgr.on<DeactiveTextureEvent>(*this);	
	}

	void TextureSystem::receive(const LoadCubemapEvent &evt) {
		auto texDict = m_objMgr->getSingletonComponent<TextureDict>(); 
		
		std::string filename[6];
		TexRef texRef;
		glGenTextures(1, &texRef.texID);
		glActiveTexture(GL_TEXTURE0);

		int width, height;
		unsigned char* image;

		glBindTexture(GL_TEXTURE_CUBE_MAP, texRef.texID);
		for (uint32_t i = 0; i < 6; i++)
		{
			image = SOIL_load_image((evt.dirpath + evt.filenames[i]).c_str(), &width, &height, 0, SOIL_LOAD_RGB);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			SOIL_free_image_data(image);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		texDict->insert({ evt.aliasname, texRef });
	}

	void TextureSystem::receive(const LoadTextureEvent &evt) {
		auto texDict = m_objMgr->getSingletonComponent<TextureDict>();
		// Load, create texture and generate mipmaps
		int width, height, channels;
		unsigned char* image = SOIL_load_image((evt.dirpath + std::string(evt.filename)).c_str(),
			&width, &height, &channels, evt.channels);
		if (evt.channels > 0) {
			channels = evt.channels;
		}
		std::cout << "SOIL: image[" << evt.filename << "] loaded, w:" << width << ", h:" << height << std::endl;

		// Load and create a texture
		TexRef texRef;
		glGenTextures(1, &texRef.texID);
		glBindTexture(GL_TEXTURE_2D, texRef.texID); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
													// Set our texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, channels == 4 ? GL_CLAMP_TO_EDGE : GL_REPEAT);	// Set texture wrapping to GL_REPEAT
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, channels == 4 ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		auto format = GL_RGB;
		auto formatWithAlpha = GL_RGBA;
		if (evt.toLinear) {
			format = GL_SRGB;
			formatWithAlpha = GL_SRGB_ALPHA;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, channels == 4 ? formatWithAlpha : format,
			width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
		texRef.width = width;
		texRef.height = height;
		texRef.type = TexType::Tex2D;
		texDict->insert({ evt.aliasname, texRef });
	}

	void TextureSystem::receive(const DestroyTextureEvent &evt) {
		auto texDict = m_objMgr->getSingletonComponent<TextureDict>();
		auto it = texDict->find(evt.aliasname);
		if (it == texDict->end()) {
			return;
		}
		glDeleteTextures(1, &it->second.texID);
		texDict->erase(it);
	}
	
	void TextureSystem::receive(const ActiveTextureByIDEvent &evt) {
		if (evt.texID > 0) {
			assert(glIsTexture(evt.texID));
			glActiveTexture(GL_TEXTURE0 + evt.idx);
			glBindTexture(GL_TEXTURE_2D, evt.texID);
			return;
		}
	}

	void TextureSystem::receive(const ActiveTextureEvent &evt) {
		if (evt.aliasname == "") {
			return;
		}
		auto texDict = m_objMgr->getSingletonComponent<TextureDict>();
		auto it = texDict->find(evt.aliasname);
		if (it == texDict->end()) {
			return;
		}
		TexRef texRef = it->second;
		glActiveTexture(GL_TEXTURE0 + evt.idx);
		if (texRef.type == TexType::Tex2D) {
			glBindTexture(GL_TEXTURE_2D, texRef.texID);
		}
		else if (texRef.type == TexType::CubeMap) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, texRef.texID);
		}
	}
    
    void TextureSystem::receive(const DeactiveTextureEvent &evt) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

};
