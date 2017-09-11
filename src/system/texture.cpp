#include "stdafx.h"
#include "system/texture.hpp"
#include "SOIL.h"

using namespace std;

namespace renderer {
	void TextureSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		evtMgr.on<LoadTextureEvent>(*this);
		evtMgr.on<DestroyTextureEvent>(*this);
		evtMgr.on<ActiveTextureEvent>(*this);
	}

	void TextureSystem::receive(const LoadTextureEvent &evt) {
		auto texDict = evt.obj.component<TextureDict>();
		
		// Load and create a texture
		TexRef texRef;
		glGenTextures(1, &texRef.texID);
		glBindTexture(GL_TEXTURE_2D, texRef.texID); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
													// Set our texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, evt.hasAlpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);	// Set texture wrapping to GL_REPEAT
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, evt.hasAlpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Load, create texture and generate mipmaps
		int width, height;
		unsigned char* image = SOIL_load_image((evt.dirpath + std::string(evt.filename)).c_str(),
			&width, &height, 0, evt.hasAlpha ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);

		std::cout << "SOIL: image[" << evt.filename << "] loaded, w:" << width << ", h:" << height << std::endl;

		auto format = GL_RGB;
		auto formatWithAlpha = GL_RGBA;
		if (evt.toLinear) {
			format = GL_SRGB;
			formatWithAlpha = GL_SRGB_ALPHA;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, evt.hasAlpha ? formatWithAlpha : format, 
			width, height, 0, evt.hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
		texRef.width = width;
		texRef.height = height;
		texRef.type = TexType::Tex2D;
		texDict->insert({ evt.aliasname, texRef });
	}

	void TextureSystem::receive(const DestroyTextureEvent &evt) {
		auto texDict = evt.obj.component<TextureDict>();
		auto it = texDict->find(evt.aliasname);
		if (it == texDict->end()) {
			return;
		}
		glDeleteTextures(1, &it->second.texID);
		texDict->erase(it);
	}

	void TextureSystem::receive(const ActiveTextureEvent &evt) {
		auto texDict = evt.obj.component<TextureDict>();
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
};
