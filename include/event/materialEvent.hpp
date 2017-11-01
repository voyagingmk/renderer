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
		LoadAiMaterialEvent(Object obj, int matNum, aiMaterial** mMaterials, std::string texDir, bool inverseNormal) :
			obj(obj),
			matNum(matNum),
			mMaterials(mMaterials),
			texDir(texDir),
			inverseNormal(inverseNormal)
		{}
		Object obj;
		int matNum;
		aiMaterial** mMaterials;
		std::string texDir;
		bool inverseNormal;
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
