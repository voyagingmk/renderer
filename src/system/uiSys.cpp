#include "stdafx.h"
#include "system/uiSys.hpp"
#include "com/sdlContext.hpp"
#include "com/spatialData.hpp"
#include "com/cameraCom.hpp"
#include "com/lightCom.hpp"
#include "com/miscCom.hpp"
#include "event/lightEvent.hpp"
#include "imgui_impl_sdl_gl3.h"

using namespace std;

namespace renderer {
    void UISystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("UISystem init\n");
        evtMgr.on<SetupSDLDoneEvent>(*this);
        evtMgr.on<DrawUIEvent>(*this);
    }
    
    void UISystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        
    }
    void UISystem::receive(const DrawUIEvent &evt) {
		auto com = m_objMgr->getSingletonComponent<SDLContext>();
        ImGui_ImplSdlGL3_NewFrame(com->win);
        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        // Rendering
		settingUI();
		lightUI();
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        ImGui::Render();
    }

	void UISystem::lightUI() {
		ImGui::Begin("Settings");
		for (auto obj : m_objMgr->entities<LightTag, LightCommon>()) {
			ImGui::PushID(obj.ID());
			if (obj.hasComponent<PointLightCom>()) {
				ImGui::Text(("PointLight" + std::to_string(obj.ID())).c_str());
				auto lightCom = obj.component<PointLightCom>();
			} else if (obj.hasComponent<DirLightCom>()) {
				ImGui::Text(("DirLight" + std::to_string(obj.ID())).c_str());
				auto spatialData = obj.component<SpatialData>();
				auto dirLightTrans = obj.component<DirLightTransform>();
				float size = dirLightTrans->size;
				float n = dirLightTrans->n;
				float f = dirLightTrans->f;
				ImGui::SliderFloat("size", &size, 0.001f, 500.0f);
				ImGui::SliderFloat("near_plane", &n, 0.001f, 500.0f);
				ImGui::SliderFloat("far_plane", &f, 0.001f, 500.0f);
				float pos[3] = { spatialData->pos.x, spatialData->pos.y, spatialData->pos.z };
				ImGui::SliderFloat3("position", pos, -500.0f, 500.0f);
				auto posV = Vector3dF(pos[0], pos[1], pos[2]);
				if (spatialData->pos != posV || dirLightTrans->size != size || dirLightTrans->n != n || dirLightTrans->f != f) {
					dirLightTrans->n = n;
					dirLightTrans->f = f;
					dirLightTrans->size = size;
					spatialData->pos = posV;
					m_evtMgr->emit<UpdateLightEvent>(obj);
				}
			} else if (obj.hasComponent<SpotLightCom>()) {
				ImGui::Text(("SpotLight" + std::to_string(obj.ID())).c_str());
				auto spatialData = obj.component<SpatialData>();
				auto lightCom = obj.component<SpotLightCom>();
				auto spotLightTrans = obj.component<SpotLightTransform>();
				auto cutOff = lightCom->cutOff.ToDegree();
				auto outerCutOff = lightCom->outerCutOff.ToDegree();
				bool changed = false;
				changed |= ImGui::SliderFloat("cutOff", &cutOff.degree, 0.001f, 180.0f);
				changed |= ImGui::SliderFloat("outerCutOff", &outerCutOff.degree, 0.001f, 180.0f);
				lightCom->cutOff = cutOff.ToRadian();
				if (outerCutOff.degree < cutOff.degree) {
					outerCutOff.degree = cutOff.degree;
				}
				lightCom->outerCutOff = outerCutOff.ToRadian();
				float n = spotLightTrans->n;
				float f = spotLightTrans->f;
				changed |= ImGui::SliderFloat("near_plane", &n, 0.001f, 10000.0f);
				changed |= ImGui::SliderFloat("far_plane", &f, 0.001f, 10000.0f);
				float pos[3] = { spatialData->pos.x, spatialData->pos.y, spatialData->pos.z };
				changed |= ImGui::SliderFloat3("position", pos, -1000.0f, 1000.0f);
				float dir[3] = { lightCom->direction.x, lightCom->direction.y, lightCom->direction.z };
				changed |= ImGui::SliderFloat3("direction", dir, -1.0f, 1.0f);
				auto posV = Vector3dF(pos[0], pos[1], pos[2]);
				auto dirV = Vector3dF(dir[0], dir[1], dir[2]);
				if (changed) {
					spotLightTrans->n = n;
					spotLightTrans->f = f;
					lightCom->direction = dirV.Normalize();
					spatialData->pos = posV;
					m_evtMgr->emit<UpdateLightEvent>(obj);
				}
			}
			auto lightCommon = obj.component<LightCommon>();
			ImGui::SliderFloat("intensity", &lightCommon->intensity, 0.001f, 50.0f);
			ImGui::ColorEdit3("ambient", lightCommon->ambient.rgba);
			ImGui::PopID();
		}
		ImGui::End();
	}

	void UISystem::settingUI() {
		auto com = m_objMgr->getSingletonComponent<SDLContext>();
		auto cameraView = m_objMgr->getSingletonComponent<PerspectiveCameraView>();
		ImGui::Begin("Settings");
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
		ImGui::Text("ShadowMap:");

		auto gSettingCom = m_objMgr->getSingletonComponent<GlobalSettingCom>();

		float depthBias = gSettingCom->getValue("depthBias");
		ImGui::SliderFloat("depthBias", &depthBias, -6.0f, 6.0f);
		gSettingCom->setValue("depthBias", depthBias);

		float normalOffset = gSettingCom->getValue("normalOffset");
		ImGui::SliderFloat("normalOffset", &normalOffset, -3.0f, 3.0f);
		gSettingCom->setValue("normalOffset", normalOffset);

		float diskFactor = gSettingCom->getValue("diskFactor");
		ImGui::SliderFloat("diskFactor", &diskFactor, 0.0f, 30.0f);
		gSettingCom->setValue("diskFactor", diskFactor);


		float pointLightConstant = gSettingCom->getValue("pointLightConstant");
		ImGui::SliderFloat("PointLightConstant", &pointLightConstant, 0.0001f, 1.0f);
		gSettingCom->setValue("pointLightConstant", pointLightConstant);

		float pointLightLinear = gSettingCom->getValue("pointLightLinear");
		ImGui::SliderFloat("PointLightLinear", &pointLightLinear, 0.0f, 1.0f);
		gSettingCom->setValue("pointLightLinear", pointLightLinear);

		float pointLightQuad = gSettingCom->getValue("pointLightQuad");
		ImGui::SliderFloat("PointLightQuad", &pointLightQuad, 0.0f, 1.0f);
		gSettingCom->setValue("pointLightQuad", pointLightQuad);

		bool enableSSAO = gSettingCom->getValue("enableSSAO");
		ImGui::Checkbox("SSAO", &enableSSAO);
		gSettingCom->setValue("enableSSAO", enableSSAO);

		bool enableSMAA = gSettingCom->getValue("enableSMAA");
		ImGui::Checkbox("SMAA", &enableSMAA);
		gSettingCom->setValue("enableSMAA", enableSMAA);

		bool enableToneMapping = gSettingCom->getValue("enableToneMapping");
		ImGui::Checkbox("ToneMapping", &enableToneMapping);
		gSettingCom->setValue("enableToneMapping", enableToneMapping);

		bool enableGamma = gSettingCom->getValue("enableGamma");
		ImGui::Checkbox("GammaCorrect", &enableGamma);
		gSettingCom->setValue("enableGamma", enableGamma);

		bool debugShadow = gSettingCom->getValue("debugShadow");
		ImGui::Checkbox("DebugShadow", &debugShadow);
		gSettingCom->setValue("debugShadow", debugShadow);

		auto pos = cameraView->GetCameraPosition();
		ImGui::Text("Camera: %.2f %.2f %.2f", pos.x, pos.y, pos.z);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}


    void UISystem::receive(const SetupSDLDoneEvent &evt) {
        auto com = evt.obj.component<SDLContext>();
        ImGui_ImplSdlGL3_Init(com->win);
    }

};
