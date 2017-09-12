#include "stdafx.h"
#include "com/materialCom.hpp"
#include "system/materialSys.hpp"
#include "utils/helper.hpp"

using namespace std;

namespace renderer {


	void MaterialSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        evtMgr.on<LoadMaterialEvent>(*this);
    }
    

	void MaterialSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}

	void MaterialSystem::receive(const LoadMaterialEvent &evt) {
        auto com = evt.obj.getSingletonComponent<MaterialSet>();
        com->settings.insert({evt.matInfo["id"], {
            parseColor(evt.matInfo["ambient"]),
            parseColor(evt.matInfo["diffuse"]),
            parseColor(evt.matInfo["specular"]),
            evt.matInfo["shininess"],
            evt.matInfo["reflectiveness"]}});
        printf("LoadMaterial: %d\n", (int)(evt.matInfo["id"]));
    }

};
