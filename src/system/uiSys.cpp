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
        
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        // Rendering
        auto s = ImGui::GetIO().DisplaySize;
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        // glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        // glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
    }
    
    void UISystem::receive(const SetupSDLDoneEvent &evt) {
        auto com = evt.obj.component<SDLContext>();
        ImGui_ImplSdlGL3_Init(com->win);
    }

};
