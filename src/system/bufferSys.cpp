#include "stdafx.h"
#include "system/bufferSys.hpp"
#include "realtime/glutils.hpp"

using namespace std;

namespace renderer {

	void BufferSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		evtMgr.on<ComponentAddedEvent<Meshes>>(*this);
		evtMgr.on<DrawMeshBufferEvent>(*this);
	}

	void BufferSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}

	void BufferSystem::receive(const ComponentAddedEvent<Meshes> &evt) {
		Object obj = evt.m_obj;
		auto com = obj.addComponent<MeshBufferDictsCom>();
		for(const OneMesh& mesh : evt.component->meshes){
			com->dicts.push_back(CreateMeshBuffer(mesh));
		}
	}

	void BufferSystem::receive(const DrawMeshBufferEvent& evt) {
		Object obj = evt.obj; 
		auto com = obj.component<MeshBufferDictsCom>();
		for (auto meshBuffer : com->dicts) {
			glBindVertexArray(meshBuffer.vao);
			//glDrawArrays(GL_TRIANGLES, 0, meshBuffer.triangles);
			glDrawElements(GL_TRIANGLES, meshBuffer.triangles * 3, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			CheckGLError;
		}
	}

	void BufferSystem::DestroyFrameBuffer(FrameBufferBase& buf) {
		glDeleteFramebuffers(1, &buf.fboID);
	}

	void BufferSystem::UseFrameBuffer(FrameBufferBase& buf) {
		glBindFramebuffer(GL_FRAMEBUFFER, buf.fboID);
	}

	void BufferSystem::UnuseFrameBuffer(FrameBufferBase& buf) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	MeshBufferRef BufferSystem::CreateMeshBuffer(const OneMesh& mesh) {
		GLuint VBO, VAO, EBO;
		MeshBufferRef meshBuffer;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexes.size() * sizeof(unsigned int), &mesh.indexes[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		//Vertex Color attribute
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		//TexCoord attribute
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(9 * sizeof(GLfloat)));
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

		glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
		CheckGLError;

		meshBuffer.vao = VAO;
		meshBuffer.vbo = VBO;
		meshBuffer.ebo = EBO;
		assert(VAO > 0 && VBO > 0 && EBO > 0);
		meshBuffer.triangles = mesh.indexes.size() / 3;
		//bufferDict[aliasname] = meshBuffer;
		return meshBuffer;
	}

	ColorBufferRef BufferSystem::CreateColorBuffer(size_t width, size_t height, BufType depthType, size_t MSAA) {
		ColorBufferRef buf;
		buf.width = width;
		buf.height = height;
		buf.tex.width = width;
		buf.tex.height = height;
		buf.MSAA = MSAA;
		glGenFramebuffers(1, &buf.fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, buf.fboID);

		auto target = GL_TEXTURE_2D;
		if (MSAA) {
			target = GL_TEXTURE_2D_MULTISAMPLE;
		}

		// color buffer
		glGenTextures(1, &buf.tex.texID);
		glBindTexture(target, buf.tex.texID);
		if (MSAA) {
			size_t samples = MSAA;
			glTexImage2DMultisample(target, samples, GL_RGBA16F, width, height, GL_TRUE);
		}
		else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(target, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, buf.tex.texID, 0);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("[CreateFrameBuffer failed]");
			DestroyFrameBuffer(buf);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return buf;
		}
		if (MSAA) {
			// configure second post-processing framebuffer
			glGenFramebuffers(1, &buf.innerFboID);
			glBindFramebuffer(GL_FRAMEBUFFER, buf.innerFboID);
			// create a color attachment texture
			glGenTextures(1, &buf.innerTex.texID);
			glBindTexture(GL_TEXTURE_2D, buf.innerTex.texID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buf.innerTex.texID, 0);	// we only need a color buffer
		}
		glBindFramebuffer(GL_FRAMEBUFFER, buf.fboID);
		// depth and stencil buffer
		buf.depthType = depthType;
		if (buf.depthType == BufType::Tex) {
			glGenTextures(1, &buf.depthTex.texID);
			glBindTexture(GL_TEXTURE_2D, buf.depthTex.texID);
			if (MSAA) {
				size_t samples = MSAA;
				glTexImage2DMultisample(target, samples, GL_DEPTH24_STENCIL8, width, height, GL_TRUE);
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, buf.depthTex.texID, 0);
		}
		else {
			glGenRenderbuffers(1, &buf.depthRboID);
			glBindRenderbuffer(GL_RENDERBUFFER, buf.depthRboID);
			if (MSAA) {
				size_t samples = MSAA;
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
			}
			else {
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			}
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buf.depthRboID);
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("[CreateFrameBuffer failed]");
			DestroyFrameBuffer(buf);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return buf;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return buf;
	}

	GBufferRef BufferSystem::CreateGBuffer(size_t width, size_t height) {
		GBufferRef buf;
		buf.width = width;
		buf.height = height;
		// configure g-buffer framebuffer
		glGenFramebuffers(1, &buf.fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, buf.fboID);
		unsigned int gPosition, gNormal, gAlbedoSpec;
		// position color buffer
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
		// normal color buffer
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
		// color + specular color buffer
		glGenTextures(1, &gAlbedoSpec);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);
		// create and attach depth buffer (renderbuffer)
		glGenRenderbuffers(1, &buf.depthRboID);
		glBindRenderbuffer(GL_RENDERBUFFER, buf.depthRboID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buf.depthRboID);
		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return buf;
	}
};
