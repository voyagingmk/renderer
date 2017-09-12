#include "stdafx.h"
#include "system/materialSys.hpp"
#include "utils/helper.hpp"

using namespace std;

namespace renderer {


	void MaterialSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        evtMgr.on<LoadMaterialEvent>(*this);
		evtMgr.on<ActiveMaterialEvent>(*this);
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
            evt.matInfo["reflectiveness"],
			evt.matInfo["shaderName"] }});
        printf("LoadMaterial: %d\n", (int)(evt.matInfo["id"]));
    }

	void MaterialSystem::receive(const ActiveMaterialEvent &evt) {
		ObjectManager& objMgr = evt.obj.objMgr();
		auto matSetCom = objMgr.getSingletonComponent<MaterialSet>();
		auto mat = matSetCom->settings[evt.settingID];
		auto spSeCom = objMgr.getSingletonComponent<ShaderProgramSet>();
		Shader shader(spSeCom->alias2HDL[mat.shaderName]);
		activeMaterial(shader, mat);
	}

	void MaterialSystem::activeMaterial(Shader& shader, MaterialSettingCom& setting) {
		shader.set3f("material.ambient", setting.ambient.r(), setting.ambient.g(), setting.ambient.b());
		shader.set3f("material.diffuse", setting.diffuse.r(), setting.diffuse.g(), setting.diffuse.b());
		shader.set3f("material.specular", setting.specular.r(), setting.specular.g(), setting.specular.b());
		shader.set1f("material.shininess", setting.shininess);
	}

};
