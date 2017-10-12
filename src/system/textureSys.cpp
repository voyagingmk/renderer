#include "stdafx.h"
#include "system/textureSys.hpp"
#include "realtime/glutils.hpp"
#include "SOIL.h"
#include "utils/helper.hpp"
#include "com/glcommon.hpp"

using namespace std;

namespace renderer {
	void TextureSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("TextureSystem init\n");
        evtMgr.on<LoadTextureEvent>(*this);
        evtMgr.on<LoadCubemapEvent>(*this);
        evtMgr.on<CreateNoiseTextureEvent>(*this);
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

		int width, height, channels;

		glBindTexture(GL_TEXTURE_CUBE_MAP, texRef.texID);
		for (uint32_t i = 0; i < 6; i++)
		{
			unsigned char* image = SOIL_load_image((evt.dirpath + evt.filenames[i]).c_str(), &width, &height, &channels, SOIL_LOAD_RGB);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			SOIL_free_image_data(image);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		texRef.type = TexType::CubeMap;

		texDict->insert({ evt.aliasname, texRef });
		CheckGLError;
		std::cout << "TextureSystem: cubemap[" << evt.aliasname << "] loaded, w:" << width << ", h:" << height << std::endl;
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
		CheckGLError;
		std::cout << "TextureSystem: image[" << evt.filename << "] loaded, w:" << width << ", h:" << height << std::endl;

	}
    
    void TextureSystem::receive(const CreateNoiseTextureEvent &evt) {
        auto texDict = m_objMgr->getSingletonComponent<TextureDict>();
        // Noise texture
        std::vector<Vector3dF> ssaoNoise;
        for (GLuint i = 0; i < 16; i++)
        {
            Vector3dF noise(random0_1<float>() * 2.0 - 1.0, random0_1<float>() * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
            ssaoNoise.push_back(noise);
        }
        TexRef texRef;
        glGenTextures(1, &texRef.texID);
        glBindTexture(GL_TEXTURE_2D, texRef.texID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        texRef.width = 4;
        texRef.height = 4;
        texRef.type = TexType::Tex2D;
        texDict->insert({ evt.aliasname, texRef });
		CheckGLError;
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
        assert(glIsTexture(evt.texID) &&  evt.texID > 0);
        glActiveTexture(GL_TEXTURE0 + evt.idx);
        Shader shader = evt.shader;
        shader.set1i(evt.sample2DName.c_str(), (int)(evt.idx));
        glBindTexture(GL_TEXTURE_2D, evt.texID);
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
        Shader shader = evt.shader;
        shader.set1i(evt.sample2DName.c_str(), (int)(evt.idx));
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
