#include "stdafx.h"
#include "system/uiSys.hpp"
#include "com/sdlContext.hpp"
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
        {
            static float f = 0.0f;
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
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
