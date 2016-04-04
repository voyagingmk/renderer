#ifndef RENDERER_PARSER_HPP
#define RENDERER_PARSER_HPP

#include "base.hpp"


namespace renderer {
	class Film;

	class Parser {
	public:
		int parseFromJson(nlohmann::json config, Film * film);
	private:
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
		Color parseColor(std::string c);
		void parserObj(std::string inputfile);
	};
}


#endif //PARSER_HPP
