#ifndef RENDERER_PARSER_HPP
#define RENDERER_PARSER_HPP

#include "base.hpp"


namespace renderer {
	class Parser {
	public:
		int parseFromFile(std::string path);
	private:
		PtrColor parseColor(std::string c);
	};
}


#endif //PARSER_HPP
