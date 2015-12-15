#include "stdafx.h"
#include "parser.hpp"
#include "tiny_obj_loader.h"
#include "transform.hpp"
using namespace renderer;


int main(int argc, char ** argv){
	Parser parser;
	//parser.parseFromFile("config.json");
	Matrix4x4 m1 = { 
		1.f,0,0,0,
		0,1.f,0,0,
		0,0,1.f,0,
		0,0,0,1.f };
	Matrix4x4 m2 = { 
		1.f,2.f,3.f,4.f,
		5.f,6.f,7.f,8.f,
		1.f,2.f,3.f,4.f,
		5.f,6.f,7.f,8.f };
	Matrix4x4 m3 = m1.add(m2);
	Matrix4x4 m4 = m3.minus(m2);
	m4.debug();
	auto m5 = m1.multiply(m2.multiply(3.0f).add(m2).minus(m2).divide(3.0f));
	m5.debug();
	auto m6 = m5.clone();
	m6.debug();
	m6.transpose().debug();
	Matrix4x4 m7 = {
		1.f,1.f,1.f,1.f,
		1.f,1.f,1.f,1.f,
		1.f,1.f,1.f,1.f,
		1.f,1.f,1.f,1.f,
	};
	m7.power(1).debug();
	return 0;
}
