#ifndef RENDERER_COM_MISC_HPP
#define RENDERER_COM_MISC_HPP

#include "base.hpp"
#include "ecs/setting.hpp"
#include "com/matrix.hpp"
#include "com/materialCom.hpp"
#include "com/mesh.hpp"

namespace renderer {
    
	struct TagComBase {};
	
	struct LightTag : public TagComBase {};

    struct GlobalSkyboxTag : public TagComBase {};
    
	struct ReceiveLightTag : public TagComBase {};

	struct RenderableTag : public TagComBase {};

	struct StaticObjTag : public TagComBase {}; // 不能直接渲染，要先batch化

	struct DynamicObjTag : public TagComBase {};
    
    struct DynamicBatchObjTag : public TagComBase {};

	struct RootNodeTag : public TagComBase {};

    
	typedef int BufIdx;
	typedef std::vector<std::tuple<ecs::ObjectID, ecs::ObjectID, BufIdx>> RenderQueue;

	struct StaticRenderQueueCom {
		RenderQueue queue;
	};

	struct DynamicRenderQueueCom {
		RenderQueue queue;
	};
    
    // 记录这个batch管理的obj和用于instance的信息
    struct BatchInfoCom {
		MeshID meshID;
		std::string bufferName;
        // 长度一致
        std::vector<ecs::ObjectID> objIDs;
        std::vector<Matrix4x4Value> modelMatrixes;
    };

	struct BatchObjectListCom {
		std::vector<ecs::ObjectID> objIDs;
	};

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
			return (T)(_getValue(k, defaultVal));
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
