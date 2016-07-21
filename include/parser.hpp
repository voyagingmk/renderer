#ifndef RENDERER_PARSER_HPP
#define RENDERER_PARSER_HPP

#include "base.hpp"
#include "renderer.hpp"

namespace renderer {
	class Film;

	class SceneParser {
	public:
		SceneDesc parse(nlohmann::json& config);
	private:
		SceneDesc parseSceneDesc(nlohmann::json& config);
		PerspectiveCamera parsePerspectiveCamera(nlohmann::json& config);
		void parseMaterials(nlohmann::json& config, MaterialDict& matDict);
		Transform* parseTransform(nlohmann::json& config);
		ShapeUnion parseShapes(nlohmann::json& config, MaterialDict& matDict);
		void parseLights(nlohmann::json& config, Lights& lights, MaterialDict& matDict);
		Color parseColor(std::string c);
		void parserObj(std::string inputfile);
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
