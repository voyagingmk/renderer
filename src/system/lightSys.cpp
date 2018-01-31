#include "stdafx.h"
#include "system/lightSys.hpp"
#include "com/spatialData.hpp"
#include "event/bufferEvent.hpp"
#include "event/shaderEvent.hpp"
#include "com/bufferCom.hpp"

using namespace std;

namespace renderer {
    void LightSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("LightSystem init\n");
        evtMgr.on<UpdateLightEvent>(*this);
        evtMgr.on<AddLightEvent>(*this);
		evtMgr.on<UpdateSpatialDataEvent>(*this);
		evtMgr.on<EnableLightShadowEvent>(*this);
		evtMgr.on<DisableLightShadowEvent>(*this);
        evtMgr.on<DrawLightBoundEvent>(*this);
    }
    
    void LightSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        
    }
    
    void LightSystem::receive(const DrawLightBoundEvent &evt) {
        Object obj = evt.obj;
        auto spatialData = obj.component<SpatialData>();
        auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
        Shader shader = spSetCom->getShader("wireframe");
        shader.use();
        shader.set3f("wireColor", Vector3dF{ 1.0f, 1.0f, 0.0f});
        shader.set1b("instanced", false);
        m_evtMgr->emit<UploadCameraToShaderEvent>(evt.objCamera, shader);
        m_evtMgr->emit<CreateDynamicMeshBufferEvent>("wirelight", true);
        m_evtMgr->emit<BindDynamicMeshBufferEvent>("wirelight");
        shader.setMatrix4f("modelMat", Matrix4x4::newIdentity());
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if (obj.hasComponent<PointLightTransform>()) {
            auto com = obj.component<PointLightTransform>();
            Vector3dF lightPos = spatialData->pos;
            std::vector<Vertex> vertices;
            const int horizontalLines = 20;
            const int verticalLines = 20;
            const float radius = com->f;
            for (int m = 0; m < horizontalLines; m++)
            {
                for (int n = 0; n < verticalLines - 1; n++)
                {
                    float x = sin(M_PI * m / horizontalLines) * cos(2 * M_PI * n/verticalLines);
                    float y = sin(M_PI * m / horizontalLines) * sin(2 * M_PI * n/verticalLines);
                    float z = cos(M_PI * m / horizontalLines);
                    auto p = lightPos + Vector3dF(x, y, z) * radius;
                    vertices.push_back({p});
                }
            }
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STREAM_DRAW);
            glDrawArrays(GL_LINE_LOOP, 0, vertices.size());
        }
        else if (obj.hasComponent<DirLightTransform>()) {
            Vector3dF lightPos = spatialData->pos;
            std::vector<Vertex> vertices;
            for (int i = -5; i < 5; i++) {
                Vector3dF o = {i * 3.0f, 0, 0};
                Vector3dF p = o + lightPos;
                vertices.push_back({o});
                vertices.push_back({p});
            }
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STREAM_DRAW);
            glDrawArrays(GL_LINES, 0, vertices.size());
        }
        else if (obj.hasComponent<SpotLightTransform>()) {
            auto com = obj.component<SpotLightTransform>();
            auto spotLightCom = obj.component<SpotLightCom>();
            Vector3dF lightPos = spatialData->pos;
            std::vector<Vertex> vertices;
            auto d = spotLightCom->direction;
            // from light position to far plane center position
            auto c = lightPos + spotLightCom->direction * com->f; // farPlaneCenter
            // vertices.push_back({lightPos});
            // vertices.push_back({c});
            auto randomV = spotLightCom->direction;
            Vector3dF p(30, 40, 50);
            if (d.x != 0.0f) {
                p.x = -(d.z * (p.z - c.z) + d.y * (p.y - c.y)) / d.x + c.x;
            } else if (d.y != 0.0f) {
                p.y = -(d.x * (p.x - c.x) + d.z * (p.z - c.z)) / d.y + c.y;
            } else if (d.z != 0.0f) {
                p.z = -(d.x * (p.x - c.x) + d.y * (p.y - c.y)) / d.z + c.z;
            }
            p = (p - c).Normalize() * spotLightCom->calOuterRadius(com->f);
			Vector3dF pOld = p;
            // from light position to far plane contour
            for (float theta = 30.0f; theta <= 360.0f; theta += 30.0f) {
				float r = Radians(0.5f * theta);
				Vector3dF v = d * sin(r);
				QuaternionF tmp(0.0f, p.x, p.y, p.z);
				QuaternionF q(cos(r), v.x, v.y, v.z);
				tmp = tmp.Rotate(q);
				Vector3dF pNew = { tmp.x, tmp.y, tmp.z };
				vertices.push_back({ lightPos });
				vertices.push_back({ c + pNew });
				vertices.push_back({ c + pOld });
				vertices.push_back({ c + pNew });
				pOld = pNew;
            }
 
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STREAM_DRAW);
            glDrawArrays(GL_LINES, 0, vertices.size());
        }
        m_evtMgr->emit<UnbindDynamicMeshBufferEvent>("wirelight");
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void LightSystem::receive(const UpdateLightEvent &evt) {
		updateLight(evt.obj);
    }

    void LightSystem::receive(const AddLightEvent &evt) {  
		updateLight(evt.obj);
    }

	void LightSystem::receive(const UpdateSpatialDataEvent &evt) {
		updateLight(evt.obj);
	}

	void LightSystem::updateLight(Object obj) {
		if (obj.hasComponent<PointLightTransform>()) {
			updatePointLight(obj);
		}
		else if (obj.hasComponent<DirLightTransform>()) {
			updateDirLight(obj);
		}
		else if (obj.hasComponent<SpotLightTransform>()) {
			updateSpotLight(obj);
		}
	}

	void LightSystem::updateSpotLight(Object obj) {
		auto spotLightCom = obj.component<SpotLightCom>();
		auto spatialData = obj.component<SpatialData>();
		auto com = obj.component<SpotLightTransform>();
		Matrix4x4 shadowProj;
		shadowProj = Perspective(spotLightCom->outerCutOff.ToDegree().degree * 2.0f, com->aspect, com->n, com->f);
		Vector3dF lightPos = spatialData->pos;
		com->lightPV = shadowProj * LookAt(lightPos, lightPos + spotLightCom->direction.Normalize(), { 0.0, 1.0, 0.0 });  // right
	}

	void LightSystem::updatePointLight(Object obj) {
		auto spatialData = obj.component<SpatialData>();
		auto com = obj.component<PointLightTransform>();
		Matrix4x4 shadowProj;
		shadowProj = Perspective(90.0f, com->aspect, com->n, com->f);
		Vector3dF lightPos = spatialData->pos;
		if (com->lightPVs.size() == 0) {
			com->lightPVs.resize(6);
		}
		com->lightPVs[0] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 });  // right
		com->lightPVs[1] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ -1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }); // left
		com->lightPVs[2] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 });   // top
		com->lightPVs[3] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 0.0, -1.0, 0.0 }, { 0.0, 0.0, -1.0 }); // bottom
		com->lightPVs[4] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 0.0, 0.0, 1.0 }, { 0.0, -1.0, 0.0 });  // near
		com->lightPVs[5] = shadowProj * LookAt(lightPos, lightPos + Vector3dF{ 0.0, 0.0, -1.0 }, { 0.0, -1.0, 0.0 }); // far
	}

	void LightSystem::updateDirLight(Object obj) {
		auto spatialData = obj.component<SpatialData>();
		auto com = obj.component<DirLightTransform>();
		Matrix4x4 shadowProj = Ortho(-com->size, com->size, -com->size, com->size, com->n, com->f);
		Vector3dF lightPos = spatialData->pos;
		com->lightPV = shadowProj * LookAt(lightPos, Vector3dF{ 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 });
	}


	void LightSystem::receive(const EnableLightShadowEvent &evt) {
		auto aliasname = "lightDepth" + std::to_string(evt.obj.ID());
        m_evtMgr->emit<DestroyColorBufferEvent>(aliasname.c_str());
        auto lightCommon = evt.obj.component<LightCommon>();
        size_t shadowMapSize = 2048;
		if (evt.obj.hasComponent<PointLightCom>()) {
            if (lightCommon->shadowType == ShadowType::Standard) {
                m_evtMgr->emit<CreateDpethBufferEvent>(
                    aliasname.c_str(),
                    aliasname.c_str(),
                    DepthTexType::CubeMap,
                    shadowMapSize);
            }
		} else if (evt.obj.hasComponent<DirLightCom>()) {
            if (lightCommon->shadowType == ShadowType::Standard) {
                m_evtMgr->emit<CreateDpethBufferEvent>(
                    aliasname.c_str(),
                    aliasname.c_str(),
                    DepthTexType::DepthOnly,
                    shadowMapSize);
            } else if (lightCommon->shadowType == ShadowType::VSM) {
                m_evtMgr->emit<CreateColorBufferEvent>(
                   shadowMapSize, shadowMapSize,
                   GL_RG32F, GL_RG, GL_FLOAT,
                   BufType::Tex, 0, GL_LINEAR, aliasname.c_str());
            }
		} else if (evt.obj.hasComponent<SpotLightCom>()) {
            if (lightCommon->shadowType == ShadowType::Standard) {
                m_evtMgr->emit<CreateDpethBufferEvent>(
                    aliasname.c_str(),
                    aliasname.c_str(),
                    DepthTexType::DepthOnly,
                    shadowMapSize);
            } else if (lightCommon->shadowType == ShadowType::VSM) {
				m_evtMgr->emit<CreateColorBufferEvent>(
					shadowMapSize, shadowMapSize,
					GL_RG32F, GL_RG, GL_FLOAT,
					BufType::Tex, 0, GL_LINEAR, aliasname.c_str());
			}
		}
	}

	void LightSystem::receive(const DisableLightShadowEvent &evt) {
		auto aliasname = "lightDepth" + std::to_string(evt.obj.ID());
		m_evtMgr->emit<DestroyColorBufferEvent>(aliasname.c_str());
	}
};
