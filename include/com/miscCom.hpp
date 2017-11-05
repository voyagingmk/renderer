#ifndef RENDERER_COM_MISC_HPP
#define RENDERER_COM_MISC_HPP

#include "base.hpp"

namespace renderer {
    
	struct TagComBase {};

	struct GlobalQuadTag: public TagComBase {};


	struct SkyboxCom {
		SkyboxCom(std::string texName):
			texName(texName)
		{}
		std::string texName;
	};

    struct GlobalSkyboxTag : public TagComBase {};
    
    struct ReceiveLightTag : public TagComBase {};

	struct GlobalSettingCom {
		std::map<std::string, nlohmann::json> params;
		bool hasKey(std::string k) {
			return params.find(k) != params.end();
		}
		nlohmann::json& getValue(std::string k) {
			return params[k];
		}
		template<class T>
		void setValue(std::string k, T v) {
			params[k] = v;
		}
	};


}

#endif
