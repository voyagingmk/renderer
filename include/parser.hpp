#ifndef RENDERER_PARSER_HPP
#define RENDERER_PARSER_HPP

#include "base.hpp"


namespace renderer {
	class Film;

	class Parser {
	public:
		int parseFromJson(nlohmann::json config, Film * film);
	private:
		Color parseColor(std::string c);
		void parserObj(std::string inputfile);
	};
}


#endif //PARSER_HPP
