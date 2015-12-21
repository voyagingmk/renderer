#include "UnitTest++/UnitTest++.h"
#include "transform.hpp"

using namespace renderer;

SUITE(TestMatrix4x4)
{
	TEST(ctor)
	{
		Matrix4x4 m1 = {
			1.f,0,	0,	0,
			0,	1.f,0,	0,
			0,	0,	1.f,0,
			0,	0,	0,	1.f };
		Matrix4x4 m2 = {
			1.f,	2.f,	3.f,	4.f,
			5.f,	6.f,	7.f,	8.f,
			9.f,	10.f,	11.f,	12.f,
			13.f,	14.f,	15.f,	16.f };
		Matrix4x4 m3 = {
			1,	0,	0,	0,
			0,	1,	0,	0,
			0,	0,	1,	0,
			0,	0,	0,	1 };
	}
	TEST(add)
	{
		Matrix4x4 m1 = {
			1.f,	2.f,	3.f,	4.f,
			5.f,	6.f,	7.f,	8.f,
			9.f,	10.f,	11.f,	12.f,
			13.f,	14.f,	15.f,	16.f };
		Matrix4x4 m2 = {
			-1.f,	-2.f,	-3.f,	-4.f,
			-5.f,	-6.f,	-7.f,	-8.f,
			-9.f,	-10.f,	-11.f,	-12.f,
			-13.f,	-14.f,	-15.f,	-16.f };
		Matrix4x4 m3 = m1.add(m2);
		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE(0.f, m3[i], 0.000001);
		}
	}
	TEST(minus)
	{
		Matrix4x4 m1 = {
			1.f,	2.f,	3.f,	4.f,
			5.f,	6.f,	7.f,	8.f,
			9.f,	10.f,	11.f,	12.f,
			13.f,	14.f,	15.f,	16.f };
		Matrix4x4 m2 = {
			1.f,	2.f,	3.f,	4.f,
			5.f,	6.f,	7.f,	8.f,
			9.f,	10.f,	11.f,	12.f,
			13.f,	14.f,	15.f,	16.f };

		Matrix4x4 m3 = m1.minus(m2);

		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE(0.f, m3[i], 0.000001);
		}
	}
	TEST(divide)
	{
		Matrix4x4 m1 = {
			1.f,	2.f,	3.f,	4.f,
			5.f,	6.f,	7.f,	8.f,
			9.f,	10.f,	11.f,	12.f,
			13.f,	14.f,	15.f,	16.f };

		Matrix4x4 m2 = m1.divide(10.0f);
		float f = 0.1f;
		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE(f, m2[i], 0.000001);
			f += 0.1f;
		}
	}
	TEST(multiplyInt)
	{
		Matrix4x4 m1 = {
			1.f,	2.f,	3.f,	4.f,
			5.f,	6.f,	7.f,	8.f,
			9.f,	10.f,	11.f,	12.f,
			13.f,	14.f,	15.f,	16.f };

		Matrix4x4 m2 = m1.multiply(0);

		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE(0.f, m2[i], 0.000001);
		}

		Matrix4x4 m3 = m1.multiply(-1);
		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE( (i + 1) * -1.f, m3[i], 0.000001);
		}
	}
	TEST(multiplyMatrix)
	{
		Matrix4x4 m1 = {
			1.f,	2.f,	3.f,	4.f,
			5.f,	6.f,	7.f,	8.f,
			9.f,	10.f,	11.f,	12.f,
			13.f,	14.f,	15.f,	16.f };

		Matrix4x4 m2 = {
			1.f,0,	0,	0,
			0,	1.f,0,	0,
			0,	0,	1.f,0,
			0,	0,	0,	1.f };

		Matrix4x4 m3 = m1.multiply(m2);

		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE(m1[i], m3[i], 0.000001);
		}
	}
	TEST(operator_override)
	{
		Matrix4x4 m1 = {
			1.f,0,	0,	0,
			0,	2.f,0,	0,
			0,	0,	3.f,0,
			0,	0,	0,	4.f };

		auto m2 = m1 * (((m1 * 3.0f + m1) - m1) / 3.0f);
		
		float f = 1.f;
		for (int row = 0; row < 4; row++) {
			for (int col = 0; col < 4; col++) {
				if (row == col) {
					CHECK_CLOSE(f * f, m2[row * 4 + col], 0.000001);
					f += 1.f;
				}
				else {
					CHECK_CLOSE(0.f, m2[row * 4 + col], 0.000001);
				}
			}
		}
	}
	
	TEST(clone)
	{
		Matrix4x4 m1 = {
			1.f,0,	0,	0,
			0,	2.f,0,	0,
			0,	0,	3.f,0,
			0,	0,	0,	4.f };
		auto m2 = m1.clone();
		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE(m1[i], m2[i], 0.000001);
		}
	}
	TEST(transpose)
	{
		Matrix4x4 m1 = {
			1.f,	2.f,	3.f,	4.f,
			5.f,	6.f,	7.f,	8.f,
			9.f,	10.f,	11.f,	12.f,
			13.f,	14.f,	15.f,	16.f };
		auto m2 = m1.transpose();
		for (int row = 0; row < 4; row++) {
			for (int col = 0; col < 4; col++) {
				CHECK_CLOSE(m1[row * 4 + col], m2[col * 4 + row], 0.000001);
			}
		}
	}
	TEST(power)
	{
		Matrix4x4 m1 = {
			1.f,0,	0,	0,
			0,	2.f,0,	0,
			0,	0,	3.f,0,
			0,	0,	0,	4.f };
		auto m2 = m1.power(3);
		float f = 1.f;
		for (int row = 0; row < 4; row++) {
			for (int col = 0; col < 4; col++) {
				if (row == col) {
					CHECK_CLOSE(pow(f,3), m2[row * 4 + col], 0.000001);
					f += 1.f;
				}
				else {
					CHECK_CLOSE(0.f, m2[row * 4 + col], 0.000001);
				}
			}
		}
	}
	TEST(newIdentity) {
		Matrix4x4 m1 = {
			1.f,0,	0,	0,
			0,	1.f,0,	0,
			0,	0,	1.f,0,
			0,	0,	0,	1.f };

		Matrix4x4 m2 = Matrix4x4::newIdentity();
		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE(m1[i], m2[i], 0.000001);
		}
	}
	TEST(inverseAsTriangular) {
		Matrix4x4 m1 = {
			9.f,	0,		0,		0,
			6.f,	3.f,	0,		0,
			4.f,	9.f,	2.f,	0,
			2.f,	2.f,	3.f,	4.f };

		Matrix4x4 m2 = m1.inverseAsTriangular();

		Matrix4x4 m3 = {
			9.f,	6.f,	4.f,	2.f,
			0,		3.f,	9.f,	2.f,
			0,		0,		2.f,	3.f,
			0,		0,		0,		4.f };
		Matrix4x4 m4 = m3.inverseAsTriangular();

		/*for (int i = 0; i < 16; i++) {
			CHECK_CLOSE(m1[i], m2[i], 0.000001);
		}*/
	}
}
