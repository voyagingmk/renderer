#include "stdafx.h"
#include "system/lightSys.hpp"
#include "com/spatialData.hpp"
#include "event/bufferEvent.hpp"
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
        auto com = obj.component<DirLightTransform>();
        Vector3dF lightPos = spatialData->pos;
        Vector3dF dir = lightPos.Normalize();
        auto spSetCom = m_objMgr->getSingletonComponent<ShaderProgramSet>();
        Shader shader = spSetCom->getShader("wireframe");
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Matrix4x4 T = Translate<Matrix4x4>(lightPos);
        QuaternionF o;
        o.RotationBetweenVectors(Vector3dF(1.0, 0.0, 0.0), dir);
        Matrix4x4 R = o.toMatrix4x4();
        Matrix4x4 S = Scale<Matrix4x4>(Vector3dF(com->size, com->size, com->size));
        shader.setMatrix4f("modelMat", T * R * S);
        shader.set3f("wireColor", Vector3dF{ 1.0f, 1.0f, 0.0f});
        m_evtMgr->emit<DrawMeshBufferEvent>("wfbox", 0);
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
		com->lightPV = shadowProj * LookAt(lightPos, lightPos + spotLightCom->direction, { 0.0, 1.0, 0.0 });  // right
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
