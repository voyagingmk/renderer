#include "parser.hpp"

int main(int argc, char ** argv){
	Parser parser;
	parser.parseFromFile("config.json");
	return 0;
}
