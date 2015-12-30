#include "stdafx.h"
#include "parser.hpp"
#include "tiny_obj_loader.h"
#include "transform.hpp"
#include "geometry.hpp"
using namespace renderer;


int main(int argc, char ** argv){
	Parser parser;
	//parser.parseFromFile("config.json");
	Matrix3x3 P = {
		20.f, 20.f,		0,
		770.f, 30.f,	0,
		400.f, 780.f,	0,
	};
	Matrix3x3 B = {
		1,	0,	0,
		-2,	2,	0,
		1,	-2,	1
	};
	Matrix3x3 BP = B * P;
	typedef Matrix<MxN<float, 1, 3>> Matrix1x3;
	cil::CImg<unsigned char> img(800, 800, 1, 3);
	img.atXYZC(P[0], P[1], 0, 1) = 255;
	img.atXYZC(P[3], P[4], 0, 1) = 255;
	img.atXYZC(P[6], P[7], 0, 1) = 255;
	for (float t = 0; t < 1; t += 0.0005f) {
		Matrix1x3 T = { 1, t, t * t };
		Matrix1x3 TBP = T * BP;
		int x = int(TBP[0]);
		int y = int(TBP[1]);
		img.atXYZC(x, y, 0, 0) = 255;
	}
	img.display("");
	return 0;
}
