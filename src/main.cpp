#include "stdafx.h"
#include "parser.hpp"
using namespace renderer;

int main(int argc, char ** argv){
	Parser parser;
	parser.parseFromFile("config.json");
	return 0;
}
