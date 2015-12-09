#include "stdafx.h"
#include "parser.hpp"
#include "tiny_obj_loader.h"
using namespace renderer;

int main(int argc, char ** argv){
	Parser parser;
	parser.parseFromFile("config.json");
	return 0;
}
