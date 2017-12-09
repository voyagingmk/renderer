#include "stdafx.h"
#include "system/bufferSys.hpp"
#include "utils/glutils.hpp"
#include "com/glcommon.hpp"
#include "event/textureEvent.hpp"

using namespace std;

namespace renderer {

	void BufferSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("BufferSystem init\n");
		evtMgr.on<CreateMeshBufferEvent>(*this);
		evtMgr.on<CreateSkyboxBufferEvent>(*this);
		evtMgr.on<DrawMeshBufferEvent>(*this);
		evtMgr.on<DrawOneMeshBufferEvent>(*this);
        evtMgr.on<CreateDpethBufferEvent>(*this);
        evtMgr.on<CreateColorBufferEvent>(*this);
        evtMgr.on<DestroyColorBufferEvent>(*this);
		evtMgr.on<AddColorBufferEvent>(*this);
        evtMgr.on<UseColorBufferEvent>(*this);
        evtMgr.on<UnuseColorBufferEvent>(*this);
		evtMgr.on<CreateGBufferEvent>(*this);
		evtMgr.on<DestroyGBufferEvent>(*this);
		evtMgr.on<UseGBufferEvent>(*this); 
		evtMgr.on<UnuseGBufferEvent>(*this);
		evtMgr.on<CopyGBufferDepth2ColorBufferEvent>(*this);
		evtMgr.on<CreateInstanceBufferEvent>(*this);
		evtMgr.on<EnabledMeshBufferInstanceEvent>(*this);
		
	}

	void BufferSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}

	void BufferSystem::receive(const CreateMeshBufferEvent &evt) {
		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
		auto meshBuffersSet = m_objMgr->getSingletonComponent<MeshBuffersSet>();
		MeshID meshID = evt.meshID || meshSet->alias2id[evt.meshName];
		Mesh& mesh = meshSet->meshDict[meshID];
		meshBuffersSet->buffersDict.insert({meshID, MeshBufferRefs()});
		MeshBufferRefs& buffers = meshBuffersSet->buffersDict[meshID];
		for (const SubMesh& mesh : mesh.meshes) {
			buffers.push_back(CreateMeshBuffer(mesh));
		}
	}

	void BufferSystem::receive(const CreateSkyboxBufferEvent &evt) {
		CreateSkyboxBuffer(evt.meshID);
	}
	
	void BufferSystem::receive(const DrawMeshBufferEvent& evt) {
		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
		auto meshBuffersSet = m_objMgr->getSingletonComponent<MeshBuffersSet>();
		MeshID meshID = evt.meshID || meshSet->alias2id[evt.meshName];
		auto bufferRefs = meshBuffersSet->buffersDict[meshID];
		MeshBufferRef& bufferRef = bufferRefs[evt.subMeshIdx];
		drawMeshBuffer(bufferRef);
	}

	void BufferSystem::receive(const EnabledMeshBufferInstanceEvent& evt) {
		/*
		Object obj = evt.obj;
		auto com = obj.component<MeshBuffersCom>();
		auto comBufferDict = m_objMgr->getSingletonComponent<InstanceBufferDictCom>();
		auto it = comBufferDict->dict.find(std::string(evt.aliasName));
		if (it == comBufferDict->dict.end()) {
			return;
		}
		glBindBuffer(GL_ARRAY_BUFFER, it->second.bufID);
		for (auto meshBuffer : com->buffers) {
			EnabledMeshBufferInstance(meshBuffer, it->second);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		*/
	}


	void BufferSystem::receive(const DrawOneMeshBufferEvent& evt) {
		drawMeshBuffer(evt.buf);
	}

	void BufferSystem::receive(const CreateInstanceBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<InstanceBufferDictCom>();
		InstanceBufferRef buf;
		glGenBuffers(1, &buf.bufID);
		glBindBuffer(GL_ARRAY_BUFFER, buf.bufID);
		glBufferData(GL_ARRAY_BUFFER, evt.instanceNum * evt.perBytes, evt.data, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		com->dict[std::string(evt.aliasName)] = buf;
	}

    void BufferSystem::receive(const CreateDpethBufferEvent& evt) {
        auto com = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
        if (com->dict.find(std::string(evt.aliasName)) != com->dict.end()) {
			return;
		}
		ColorBufferRef buf = CreateDepthFrameBuffer(evt.dtType, evt.texAliasname, evt.width);
        com->dict[std::string(evt.aliasName)] = buf;
    }
    
	void BufferSystem::receive(const CreateColorBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		ColorBufferRef buf = CreateColorBuffer(
            evt.width, evt.height, evt.internalFormat, evt.format, evt.dataType,
            evt.depthType, evt.MSAA, evt.texParam);
		com->dict[std::string(evt.aliasName)] = buf;
	}

	void BufferSystem::receive(const AddColorBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		com->dict[std::string(evt.aliasName)] = evt.buf;
	}

	void BufferSystem::receive(const DestroyColorBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		auto it = com->dict.find(std::string(evt.aliasName));
		if (it == com->dict.end()) {
			return;
		}
		DestroyColorBuffer(it->second);
		com->dict.erase(it);
	}

	void BufferSystem::receive(const UseColorBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		auto it = com->dict.find(evt.aliasName);
        if (it == com->dict.end()) {
			return;
		}
		UseFrameBuffer(it->second);
	}

	void BufferSystem::receive(const UnuseColorBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		auto it = com->dict.find(evt.aliasName);
		if (it == com->dict.end()) {
			return;
		}
		UnuseFrameBuffer(it->second);
	}

	void BufferSystem::receive(const CreateGBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<GBufferDictCom>();
		GBufferRef buf = CreateGBuffer(evt.width, evt.height);
		com->dict[std::string(evt.aliasName)] = buf;
	}

	void BufferSystem::receive(const DestroyGBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<GBufferDictCom>();
		auto it = com->dict.find(evt.aliasName);
		if (it == com->dict.end()) {
			assert(false);
			return;
		}
		DestroyGBuffer(it->second);
		com->dict.erase(it);
	}

	void BufferSystem::receive(const UseGBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<GBufferDictCom>();
		auto it = com->dict.find(evt.aliasName);
		if (it == com->dict.end()) {
			assert(false);
			return;
		}
		UseFrameBuffer(it->second);
	}

	void BufferSystem::receive(const UnuseGBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<GBufferDictCom>();
		auto it = com->dict.find(evt.aliasName);
		if (it == com->dict.end()) {
			assert(false);
			return;
		}
		UnuseFrameBuffer(it->second);
	}

	void BufferSystem::receive(const CopyGBufferDepth2ColorBufferEvent& evt) {
		auto com = m_objMgr->getSingletonComponent<GBufferDictCom>();
		auto it = com->dict.find(std::string(evt.aliasName));
		if (it == com->dict.end()) {
			assert(false);
			return;
		}
		FrameBufferBase buf1 = it->second;

		auto com2 = m_objMgr->getSingletonComponent<ColorBufferDictCom>();
		auto it2 = com2->dict.find(std::string(evt.aliasName2));
		FrameBufferBase buf2;
		if (it2 != com2->dict.end()) {
			buf2 = it2->second;
		}
		CopyFrameBufferDepth(buf1, buf2);
	}

	void BufferSystem::drawMeshBuffer(const MeshBufferRef& buf) {
		glBindVertexArray(buf.vao);
		if (buf.instanced) {
			if (buf.noIndices) {
				glDrawArraysInstanced(GL_TRIANGLES, 0, buf.triangles * 3, buf.insBuf.instanceNum);
			}
			else {
				glDrawElementsInstanced(GL_TRIANGLES, buf.triangles * 3, GL_UNSIGNED_INT, 0, buf.insBuf.instanceNum);
			}
		}
		else {
			if (buf.noIndices) {
				glDrawArrays(GL_TRIANGLES, 0, buf.triangles * 3);
			}
			else {
				glDrawElements(GL_TRIANGLES, buf.triangles * 3, GL_UNSIGNED_INT, 0);
			}
		}
		glBindVertexArray(0);

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
	
	void BufferSystem::CreateInstanceBuffer(MeshBufferRef& buf, size_t insNum, void* data) {
		glBindVertexArray(buf.vao);
		glGenBuffers(1, &buf.vboIns);
		glBindBuffer(GL_ARRAY_BUFFER, buf.vboIns);
		// TODO  instance offset info struct
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3dF) * insNum, data, GL_STATIC_DRAW);
		
		// position list
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3dF), (void*)0);
		glVertexAttribDivisor(4, 1);
		// TODO
		// scale list
		// glEnableVertexAttribArray(5);
		// glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3dF), (GLvoid*)(3 * sizeof(GLfloat)));
		// glVertexAttribDivisor(5, 1);
		// orienation list
		// glEnableVertexAttribArray(6);
		// glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3dF), (GLvoid*)(6 * sizeof(GLfloat)));
		// glVertexAttribDivisor(6, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void BufferSystem::EnabledMeshBufferInstance(MeshBufferRef& buf, const InstanceBufferRef& insBuf) {
		buf.instanced = true;
		buf.insBuf = insBuf;
		glBindVertexArray(buf.vao);
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4Value), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4Value), (void*)(sizeof(Matrix4x4Value)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4Value), (void*)(2 * sizeof(Matrix4x4Value)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4x4Value), (void*)(3 * sizeof(Matrix4x4Value)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glBindVertexArray(0);
	}

	MeshBufferRef BufferSystem::CreateMeshBuffer(const SubMesh& subMesh) {
		GLuint VBO, VAO, EBO;
		MeshBufferRef meshBuffer;
		meshBuffer.settingID = subMesh.settingID;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, subMesh.vertices.size() * sizeof(Vertex), &subMesh.vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, subMesh.indexes.size() * sizeof(unsigned int), &subMesh.indexes[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		//TexCoord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		//Vertex Tangent attribute
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(8 * sizeof(GLfloat)));
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

		glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

		meshBuffer.vao = VAO;
		meshBuffer.vbo = VBO;
		meshBuffer.ebo = EBO;
		assert(VAO > 0 && VBO > 0 && EBO > 0);
		meshBuffer.triangles = subMesh.indexes.size() / 3;
		//bufferDict[aliasname] = meshBuffer;
		return meshBuffer;
	}

	MeshBufferRef BufferSystem::CreateSkyboxBuffer(MeshID meshID) {
		auto meshBuffersSet = m_objMgr->getSingletonComponent<MeshBuffersSet>();
		auto meshSet = m_objMgr->getSingletonComponent<MeshSet>();
		Mesh& mesh = meshSet->meshDict[meshID];
		SubMesh& subMesh = mesh.meshes[0];
		MeshBufferRef meshBuffer;
		glGenVertexArrays(1, &meshBuffer.vao);
		glGenBuffers(1, &meshBuffer.vbo);
		glBindVertexArray(meshBuffer.vao);
		glBindBuffer(GL_ARRAY_BUFFER, meshBuffer.vbo);
		glBufferData(GL_ARRAY_BUFFER, subMesh.vertices.size() * sizeof(Vertex), &subMesh.vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glBindVertexArray(0); 
		glBindBuffer(GL_ARRAY_BUFFER, 0); 
		meshBuffer.noIndices = true;
		meshBuffer.triangles = 12;
		meshBuffersSet->buffersDict[meshID] = { meshBuffer };
		return meshBuffer;
	}
    
    ColorBufferRef BufferSystem::CreateDepthFrameBuffer(DepthTexType dtType, std::string texAliasname, size_t width) {
        m_evtMgr->emit<CreateDepthTextureEvent>(texAliasname, dtType, width, width);
        ComponentHandle<TextureDict> texDict = m_objMgr->getSingletonComponent<TextureDict>();
        ColorBufferRef buf;
        auto it = texDict->find(texAliasname);
		assert(it != texDict->end());
        TexRef texRef = it->second;
		assert(texRef.texID > 0);
        buf.width = texRef.width;
        buf.height = texRef.height;
        buf.depthTex = texRef;
        glGenFramebuffers(1, &buf.fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, buf.fboID);
        if (dtType == DepthTexType::DepthOnly) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buf.depthTex.texID, 0);
        } else if (dtType == DepthTexType::DepthStencil) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, buf.depthTex.texID, 0);
        } else if (dtType == DepthTexType::CubeMap) {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, buf.depthTex.texID, 0);
        }
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE) {
            printf("CreateDepthFrameBuffer failed, status:%04x\n", status);
            DestroyFrameBuffer(buf);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return buf;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return buf;
    }

    
	ColorBufferRef BufferSystem::CreateColorBuffer(
            size_t width, size_t height,
            int innerFormat, int format, int dataType,
            BufType depthType, size_t MSAA, int texParam) {
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
			glTexImage2DMultisample(target, samples, innerFormat, width, height, GL_TRUE);
		}
		else {
			glTexImage2D(target, 0, innerFormat, width, height, 0, format, dataType, NULL);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParam);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParam);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, buf.tex.texID, 0);
		glBindTexture(target, 0);

		if (MSAA) {
			// configure second post-processing framebuffer
			glGenFramebuffers(1, &buf.innerFboID);
			glBindFramebuffer(GL_FRAMEBUFFER, buf.innerFboID);
			// create a color attachment texture
			glGenTextures(1, &buf.innerTex.texID);
			glBindTexture(GL_TEXTURE_2D, buf.innerTex.texID);
			glTexImage2D(GL_TEXTURE_2D, 0, innerFormat, width, height, 0, format, dataType, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParam);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParam);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buf.innerTex.texID, 0);	// we only need a color buffer
		}
		glBindFramebuffer(GL_FRAMEBUFFER, buf.fboID);
        
		// depth and stencil buffer
		buf.depthType = depthType;
        if (buf.depthType != BufType::None) {
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
        }
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			printf("CreateColorBuffer failed, status:%04x\n", status);
			DestroyFrameBuffer(buf);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return buf;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return buf;
	}

	void BufferSystem::DestroyColorBuffer(ColorBufferRef buf) {
		glDeleteTextures(1, &buf.tex.texID);
		DestroyFrameBuffer(buf);
		if (buf.MSAA) {
			glDeleteTextures(1, &buf.innerTex.texID);
			glDeleteFramebuffers(1, &buf.innerFboID);
		}
	}

	GBufferRef BufferSystem::CreateGBuffer(size_t width, size_t height) {
		GBufferRef buf;
		buf.width = width;
		buf.height = height;
		// configure g-buffer framebuffer
		glGenFramebuffers(1, &buf.fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, buf.fboID);
		// position color buffer
		glGenTextures(1, &buf.posTexID);
		glBindTexture(GL_TEXTURE_2D, buf.posTexID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buf.posTexID, 0);
		// normal color buffer
		glGenTextures(1, &buf.normalTexID);
		glBindTexture(GL_TEXTURE_2D, buf.normalTexID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buf.normalTexID, 0);
		// albedo color buffer
		glGenTextures(1, &buf.albedoTexID);
		glBindTexture(GL_TEXTURE_2D, buf.albedoTexID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buf.albedoTexID, 0);
        // pbr attribute color buffer
        glGenTextures(1, &buf.pbrTexID);
        glBindTexture(GL_TEXTURE_2D, buf.pbrTexID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, buf.pbrTexID, 0);
        
		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[] = {
			GL_COLOR_ATTACHMENT0, 
			GL_COLOR_ATTACHMENT1, 
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, attachments);
		// create and attach depth buffer (renderbuffer)
		/*
		glGenRenderbuffers(1, &buf.depthRboID);
		glBindRenderbuffer(GL_RENDERBUFFER, buf.depthRboID);        
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		// glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buf.depthRboID);
		// glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buf.depthRboID);
		*/
		
		glGenTextures(1, &buf.depthTex.texID);
		glBindTexture(GL_TEXTURE_2D, buf.depthTex.texID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, buf.depthTex.texID, 0);
		
		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return buf;
	}

	void BufferSystem::DestroyGBuffer(GBufferRef buf) {
		TexID ids[] = {
			buf.posTexID,
			buf.normalTexID,
			buf.albedoTexID,
			buf.pbrTexID };
		glDeleteTextures(4, ids);
		DestroyFrameBuffer(buf);
	}

	void BufferSystem::CopyFrameBufferDepth(FrameBufferBase& buf, FrameBufferBase& buf2) {
		if (buf2.fboID > 0) {
			// require OpenGL 4.5
			/*glBlitNamedFramebuffer(
				buf.fboID,
				buf2.fboID,
				0, 0, buf.width, buf.height,
				0, 0, buf.width, buf.height,
				GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);*/
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, buf.fboID);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buf2.fboID);
			glBlitFramebuffer(0, 0, buf.width, buf.height, 0, 0, buf.width, buf.height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, buf.fboID);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, buf.width, buf.height, 0, 0, buf.width, buf.height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
	}
};
