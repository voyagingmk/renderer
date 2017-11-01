#include "stdafx.h"
#include "system/materialSys.hpp"
#include "utils/helper.hpp"
#include "event/textureEvent.hpp"

using namespace std;

namespace renderer {


	void MaterialSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		evtMgr.on<LoadAiMaterialEvent>(*this);
        evtMgr.on<LoadMaterialEvent>(*this);
		evtMgr.on<ActiveMaterialEvent>(*this);
		evtMgr.on<DeactiveMaterialEvent>(*this);
    }
    

	void MaterialSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {

	}

	void MaterialSystem::receive(const LoadAiMaterialEvent &evt) {
		auto com = m_objMgr->getSingletonComponent<MaterialSet>();
		Object obj = evt.obj;
		ComponentHandle<MaterialCom> matCom = obj.addComponent<MaterialCom>();
		for (unsigned int i = 0; i < evt.matNum; i++) {
			const aiMaterial* pMaterial = evt.mMaterials[i];
			/*
			for (int t = aiTextureType_DIFFUSE; t <= aiTextureType_REFLECTION; t++) {
				aiString name;
				pMaterial->Get(AI_MATKEY_NAME, name);
				std::cout << "mat count: " << pMaterial->GetTextureCount(aiTextureType(t)) <<
					", type: " << t << ", name: "<< name.data <<std::endl;
			}*/
			MaterialSettingID id = ++com->idCount;
			MaterialSettingComBase* setting = new MaterialPBRSettingCom("", 0.5, 0.5);
			setting->inverseNormal = evt.inverseNormal;
			if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString Path;
				if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string fileName = Path.data;
					m_evtMgr->emit<LoadTextureEvent>(evt.texDir, fileName, fileName, 0, true);
					setting->texList.insert({ "albedoMap", fileName });
					std::cout << "MaterialSystem: albedoMap [" << fileName << "] loaded" << std::endl;
				}
			}
			if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0) {
				aiString Path;
				if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string fileName = Path.data;
					m_evtMgr->emit<LoadTextureEvent>(evt.texDir, fileName, fileName, 0, true);
					setting->texList.insert({ "normalMap", fileName });
					std::cout << "MaterialSystem: normalMap [" << fileName << "] loaded" << std::endl;
				}
			}
			com->settings.insert({ id, setting });
			matCom->settingIDs.push_back(id);
		}
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
                evt.matInfo["roughness"],
                evt.matInfo["metallic"]);
        }
        com->settings.insert({ id, setting });
		com->alias2id.insert({ evt.matInfo["alias"], id });
		for (std::string tex : evt.matInfo["textures"]) {
			setting->texList.insert({ std::string("default"), tex });
		}
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
        } else if (setting->type() == MaterialType::PBR) {
            MaterialPBRSettingCom* com = dynamic_cast<MaterialPBRSettingCom*>(setting);
			bool hasNormalMap = setting->texList.find(std::string("normalMap")) != setting->texList.end();
            shader.set1i("hasNormalMap", true);
            shader.set1f("material.metallic", com->metallic);
            shader.set1f("material.roughness", com->roughness);
            shader.set1f("material.ao", 1.0f);
        }
		shader.set1i("inverseNormal", setting->inverseNormal);
        uint32_t idx = 0;
        for (auto texInfo: setting->texList) {
            m_evtMgr->emit<ActiveTextureEvent>(shader, texInfo.first, idx, texInfo.second);
            idx++;
        }
	}

	void MaterialSystem::receive(const DeactiveMaterialEvent &evt) {
        auto com = m_objMgr->getSingletonComponent<MaterialSet>();
        MaterialSettingComBase* setting = com->settings[evt.settingID];
        uint32_t idx = 0;
		for (auto texInfo: setting->texList) {
			m_evtMgr->emit<DeactiveTextureEvent>(idx++);
		}	
	}
};
