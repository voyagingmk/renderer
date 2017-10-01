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
        MaterialSettingComBase* setting;
        if (evt.matInfo["type"] == "Phong") {
            setting = new MaterialPhongSettingCom(evt.matInfo["shaderName"],
                parseColor(evt.matInfo["ambient"]),
                parseColor(evt.matInfo["diffuse"]),
                parseColor(evt.matInfo["specular"]),
                evt.matInfo["shininess"],
                evt.matInfo["reflectiveness"]);
        } else if (evt.matInfo["type"] == "PBR") {
            setting = new MaterialPBRSettingCom(evt.matInfo["shaderName"],
                parseColor(evt.matInfo["albedo"]),
                // "ao": 1.0,
                evt.matInfo["roughness"],
                evt.matInfo["metallic"]);
        }
        com->settings.insert({evt.matInfo["id"], setting});
		for (auto tex : evt.matInfo["textures"]) {
			setting->texList.push_back(tex);
		}
        printf("LoadMaterial: %d\n", (int)(evt.matInfo["id"]));
    }

	void MaterialSystem::receive(const ActiveMaterialEvent &evt) {
        auto com = m_objMgr->getSingletonComponent<MaterialSet>();
        MaterialSettingComBase* setting = com->settings[evt.settingID];
        
		activeMaterial(const_cast<Shader&>(evt.shader), setting);
	}

	void MaterialSystem::activeMaterial(Shader& shader, MaterialSettingComBase* setting) {
        if (setting->type() == MaterialType::Phong) {
            MaterialPhongSettingCom* com = dynamic_cast<MaterialPhongSettingCom*>(setting);
            shader.set3f("material.ambient", com->ambient);
            shader.set3f("material.diffuse", com->diffuse);
            shader.set3f("material.specular", com->specular);
            shader.set1f("material.shininess", com->shininess);
        }
        uint32_t idx = 0;
        for (auto texName: setting->texList) {
            m_evtMgr->emit<ActiveTextureEvent>(idx++, texName);
        }
	}

	void MaterialSystem::receive(const DeactiveMaterialEvent &evt) {
        auto com = m_objMgr->getSingletonComponent<MaterialSet>();
        MaterialSettingComBase* setting = com->settings[evt.settingID];
        
        uint32_t idx = 0;
		for (auto texName: setting->texList) {
			m_evtMgr->emit<DeactiveTextureEvent>(idx++);
		}	
	}
};
