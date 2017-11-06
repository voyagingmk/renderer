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

		nlohmann::json getValue(std::string k) {
			return params[k];
		}

		template<class T>
		T getValue(std::string k, nlohmann::json defaultVal) {
			return (T)(getValue(k, defaultVal));
		}

		inline double get1d(std::string k, nlohmann::json defaultVal = nullptr) {
			return (double)(_getValue(k, defaultVal));
		}

		inline float get1f(std::string k, nlohmann::json defaultVal = nullptr) {
			return (float)(_getValue(k, defaultVal));
		}

		inline bool get1b(std::string k, nlohmann::json defaultVal = nullptr) {
			return (bool)(_getValue(k, defaultVal));
		}

		void setValue(std::string k, nlohmann::json val) {
			params[k] = val;
		}

	private:
		nlohmann::json _getValue(const std::string& k, const nlohmann::json& defaultVal) {
			if (!hasKey(k)) {
				params[k] = defaultVal;
			}
			return params[k];
		}
	};


}

#endif
