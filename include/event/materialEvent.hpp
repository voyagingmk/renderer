#ifndef RENDERER_EVENT_MATERIAL_HPP
#define RENDERER_EVENT_MATERIAL_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "utils/defines.hpp"
#include "com/shader.hpp"
#include "com/materialCom.hpp"

namespace renderer {
    using json = nlohmann::json;
	using namespace ecs;

	
	class LoadAiMaterialEvent : public ecs::Event<LoadAiMaterialEvent> {
	public:
		LoadAiMaterialEvent(int matNum, aiMaterial** mMaterials,
			const std::string& texDir,
			std::vector<MaterialSettingID>& settingIDs) :
			matNum(matNum),
			mMaterials(mMaterials),
			texDir(texDir),
			settingIDs(settingIDs)
		{}
		int matNum;
		aiMaterial** mMaterials;
		const std::string& texDir;
		std::vector<MaterialSettingID>& settingIDs;
	};

	class LoadMaterialEvent : public ecs::Event<LoadMaterialEvent> {
	public:
        LoadMaterialEvent(json& matInfo):
			matInfo(matInfo)
		{}
        json& matInfo;
    };

	class ActiveMaterialEvent : public ecs::Event<ActiveMaterialEvent> {
	public:
		ActiveMaterialEvent(MaterialSettingID settingID, Shader shader) :
			shader(shader),
			settingID(settingID)
		{}
		Shader shader; 
		MaterialSettingID settingID;
	}; 

	
	class DeactiveMaterialEvent : public ecs::Event<DeactiveMaterialEvent> {
	public:
		DeactiveMaterialEvent(MaterialSettingID settingID) :
			settingID(settingID)
		{}
        MaterialSettingID settingID;
	};


};

#endif
