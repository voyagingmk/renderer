#ifndef RENDERER_PARSER_HPP
#define RENDERER_PARSER_HPP

#include "base.hpp"
#include "raytracer.hpp"
#include "tiny_obj_loader.h"

namespace renderer {
	class Film;

	class SceneParser {
	public:
		SceneDesc parse(nlohmann::json& config);

		SceneDesc parseSceneDesc(nlohmann::json& config);
		PerspectiveCamera parsePerspectiveCamera(nlohmann::json& config);
        void parseShaders(nlohmann::json& config);
        void parseTextures(nlohmann::json& config);
		void parseMaterials(nlohmann::json& config);
		Transform* parseTransform(nlohmann::json& config);
		ShapeUnion* parseShapes(nlohmann::json& config);
		void parseLights(nlohmann::json& config, Lights& lights);
		Color parseColor(nlohmann::json& c);
		template<typename T>
		Color parseColor(T& colorInfo, const Color& defaultColor) {
			if (!colorInfo.is_null()) {
				if (colorInfo.is_string())
					return parseColor(colorInfo);
				else
					return Color(colorInfo[0], colorInfo[1], colorInfo[2]);
			}
			return defaultColor;
		};
	};
}


#endif //PARSER_HPP
