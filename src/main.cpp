#include "stdafx.h"
#include "parser.hpp"
#include "tiny_obj_loader.h"
#include "transform.hpp"
using namespace renderer;


int main(int argc, char ** argv){
	Parser parser;
	//parser.parseFromFile("config.json");
	Matrix4x4 m1 = { 
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1 };
	Matrix4x4 m2 = { 
		1,2,3,4,
		5,6,7,8,
		1,2,3,4,
		5,6,7,8 };
	Matrix4x4 m3 = m1.add(m2);
	Matrix4x4 m4 = m3.minus(m2);
	m4.debug();
	auto m5 = m1.multiply(m2.multiply(3.0f));
	m5.debug();
	return 0;
}
