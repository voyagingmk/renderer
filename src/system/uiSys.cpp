#include "stdafx.h"
#include "system/uiSys.hpp"
#include "com/sdlContext.hpp"
#include "com/cameraCom.hpp"
#include "com/miscCom.hpp"
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
		auto cameraView = m_objMgr->getSingletonComponent<PerspectiveCameraView>();
        ImGui_ImplSdlGL3_NewFrame(com->win);
        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
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
			ImGui::Checkbox("enableSSAO", &enableSSAO);
			gSettingCom->setValue("enableSSAO", enableSSAO);
		
			bool enableSMAA = gSettingCom->getValue("enableSMAA");
			ImGui::Checkbox("enableSMAA", &enableSMAA);
			gSettingCom->setValue("enableSMAA", enableSMAA);


			auto pos = cameraView->GetCameraPosition();
			ImGui::Text("Camera: %.2f %.2f %.2f", pos.x, pos.y, pos.z);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        ImGui::Render();
    }


    void UISystem::receive(const SetupSDLDoneEvent &evt) {
        auto com = evt.obj.component<SDLContext>();
        ImGui_ImplSdlGL3_Init(com->win);
    }

};
