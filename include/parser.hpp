#ifndef RENDERER_PARSER_HPP
#define RENDERER_PARSER_HPP

#include "base.hpp"


namespace renderer {
	class Parser {
	public:
		int parseFromFile(std::string path);
	private:
		Color parseColor(std::string c);
		void parserObj(std::string inputfile);
	};
}


#endif //PARSER_HPP
