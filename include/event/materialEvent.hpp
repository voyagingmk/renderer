#ifndef RENDERER_EVENT_MATERIAL_HPP
#define RENDERER_EVENT_MATERIAL_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "utils/defines.hpp"
#include "com/shader.hpp"
#include "com/materialCom.hpp"

namespace renderer {
    using json = nlohmann::json;

	class LoadMaterialEvent : public ecs::Event<LoadMaterialEvent> {
	public:
        LoadMaterialEvent(json& matInfo):
			matInfo(matInfo)
		{}
        json& matInfo;
    };

	class ActiveMaterialEvent : public ecs::Event<ActiveMaterialEvent> {
	public:
		ActiveMaterialEvent(MaterialSettingCom& setting, Shader shader) :
			shader(shader),
			setting(setting)
		{}
		Shader shader; 
		MaterialSettingCom& setting;
	}; 

	
	class DeactiveMaterialEvent : public ecs::Event<DeactiveMaterialEvent> {
	public:
		DeactiveMaterialEvent(MaterialSettingCom& setting) :
			setting(setting)
		{}
		MaterialSettingCom& setting;
	};


};

#endif
