#ifndef RENDERER_EVENT_MATERIAL_HPP
#define RENDERER_EVENT_MATERIAL_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "utils/defines.hpp"

namespace renderer {
    using json = nlohmann::json;

	class LoadMaterialEvent : public ecs::Event<LoadMaterialEvent> {
	public:
        LoadMaterialEvent(ecs::Object obj,
        json& matInfo):
			obj(obj),
			matInfo(matInfo)
		{}
        ecs::Object obj;
        json& matInfo;
    };

	class ActiveMaterialEvent : public ecs::Event<ActiveMaterialEvent> {
	public:
		ActiveMaterialEvent(ecs::Object obj,
			MaterialSettingID settingID) :
			obj(obj),
			settingID(settingID)
		{}
		ecs::Object obj;
		MaterialSettingID settingID;
	}; 

};

#endif
