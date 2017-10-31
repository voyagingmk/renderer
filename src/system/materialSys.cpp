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
		MaterialSettingID id = ++com->idCount;
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
        com->settings.insert({ evt.matInfo["alias"], setting });
		com->id2alias.insert({ id, evt.matInfo["alias"] });
		for (auto tex : evt.matInfo["textures"]) {
			setting->texList.push_back(tex);
		}
    }

	void MaterialSystem::receive(const ActiveMaterialEvent &evt) {
        auto com = m_objMgr->getSingletonComponent<MaterialSet>();
        MaterialSettingComBase* setting = com->settings[com->id2alias[evt.settingID]];
		activeMaterial(const_cast<Shader&>(evt.shader), setting);
	}

	void MaterialSystem::activeMaterial(Shader& shader, MaterialSettingComBase* setting) {
        if (setting->type() == MaterialType::Phong) {
            MaterialPhongSettingCom* com = dynamic_cast<MaterialPhongSettingCom*>(setting);
            shader.set3f("material.ambient", com->ambient);
            shader.set3f("material.diffuse", com->diffuse);
            shader.set3f("material.specular", com->specular);
            shader.set1f("material.shininess", com->shininess);
        } else if (setting->type() == MaterialType::PBR) {
            MaterialPBRSettingCom* com = dynamic_cast<MaterialPBRSettingCom*>(setting);
            shader.set3f("material.albedo", com->albedo);
            shader.set1f("material.metallic", com->metallic);
            shader.set1f("material.roughness", com->roughness);
            shader.set1f("material.ao", 1.0f);
        }
        uint32_t idx = 0;
        for (auto texName: setting->texList) {
            m_evtMgr->emit<ActiveTextureEvent>(shader, "texture" + std::to_string(idx+1), idx, texName);
            idx++;
        }
	}

	void MaterialSystem::receive(const DeactiveMaterialEvent &evt) {
        auto com = m_objMgr->getSingletonComponent<MaterialSet>();
        MaterialSettingComBase* setting = com->settings[com->id2alias[evt.settingID]];
        uint32_t idx = 0;
		for (auto texName: setting->texList) {
			m_evtMgr->emit<DeactiveTextureEvent>(idx++);
		}	
	}
};
