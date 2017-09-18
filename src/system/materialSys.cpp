#include "stdafx.h"
#include "system/materialSys.hpp"
#include "utils/helper.hpp"
#include "event/textureEvent.hpp"

using namespace std;

namespace renderer {


	void MaterialSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        evtMgr.on<LoadMaterialEvent>(*this);
		evtMgr.on<ActiveMaterialEvent>(*this);
		evtMgr.on<DeactiveMaterialEvent>(*this);
    }
    

	void MaterialSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}

	void MaterialSystem::receive(const LoadMaterialEvent &evt) {
        auto com = m_objMgr->getSingletonComponent<MaterialSet>();
        com->settings.insert({evt.matInfo["id"], {
            parseColor(evt.matInfo["ambient"]),
            parseColor(evt.matInfo["diffuse"]),
            parseColor(evt.matInfo["specular"]),
            evt.matInfo["shininess"],
            evt.matInfo["reflectiveness"],
			evt.matInfo["shaderName"]}});
		MaterialSettingCom& setting = com->settings[evt.matInfo["id"]];
		for (auto tex : evt.matInfo["textures"]) {
			setting.texList.push_back(tex);
		}
        printf("LoadMaterial: %d\n", (int)(evt.matInfo["id"]));
    }

	void MaterialSystem::receive(const ActiveMaterialEvent &evt) {
		activeMaterial(const_cast<Shader&>(evt.shader), const_cast<MaterialSettingCom&>(evt.setting));
		uint32_t idx = 0;
		for (auto texName: evt.setting.texList) {
			m_evtMgr->emit<ActiveTextureEvent>(idx, texName);
		}
	}

	void MaterialSystem::activeMaterial(Shader& shader, MaterialSettingCom& setting) {
		shader.set3f("material.ambient", setting.ambient);
		shader.set3f("material.diffuse", setting.diffuse);
		shader.set3f("material.specular", setting.specular);
		shader.set1f("material.shininess", setting.shininess);
	}

	void MaterialSystem::receive(const DeactiveMaterialEvent &evt) {
		uint32_t idx = 0;
		for (auto texName: evt.setting.texList) {
			m_evtMgr->emit<DeactiveTextureEvent>(idx);
		}	
	}
};
