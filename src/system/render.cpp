#include "stdafx.h"
#include "system/render.hpp"
#include "com/meshes.hpp"

using namespace std;

namespace renderer {
    void RenderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("RenderSystem init\n");
    }
    
    void RenderSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        for(auto meshes: objMgr.entities<Meshes>()) {
            
        }
	}
};
