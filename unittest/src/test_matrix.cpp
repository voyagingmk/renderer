#include "UnitTest++/UnitTest++.h"
#include "transform.hpp"

using namespace renderer;

constexpr float  precision = 0.00001f; 
//constexpr double  precision = 0.00000001;

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
			CHECK_CLOSE(0.f, m3[i], precision);
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
			CHECK_CLOSE(0.f, m3[i], precision);
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
		Matrix4x4::V f = 0.1;
		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE(f, m2[i], precision);
			f += 0.1;
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
			CHECK_CLOSE(0.f, m2[i], precision);
		}

		Matrix4x4 m3 = m1.multiply(-1);
		for (int i = 0; i < 16; i++) {
			CHECK_CLOSE( (i + 1) * -1.f, m3[i], precision);
		}
	}
	TEST(multiplyMatrix1)
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
			CHECK_CLOSE(m1[i], m3[i], precision);
		}
	}
	TEST(multiplyMatrix2)
	{
		Matrix3x3 m1 = {
			5.f,	3.f,	5.f,
			4.f,	8.f,	8.f,
			6.f,	6.f,	7.f};

		Matrix3x3 m2 = {
			4.f,	3.f,	4.f,
			8.f,	2.f,	3.f,
			4.f,	5.f,	6.f};

		Matrix3x3 m3 = m1.multiply(m2);

		Matrix3x3 m4 = {
			64.f,	46.f,	59.f,
			112.f,	68.f,	88.f,
			100.f,	65.f,	84.f };

		for (int i = 0; i < 9; i++) {
			CHECK_CLOSE(m4[i], m3[i], precision);
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
					CHECK_CLOSE(f * f, m2[row * 4 + col], precision);
					f += 1.f;
				}
				else {
					CHECK_CLOSE(0.f, m2[row * 4 + col], precision);
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
			CHECK_CLOSE(m1[i], m2[i], precision);
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
				CHECK_CLOSE(m1[row * 4 + col], m2[col * 4 + row], precision);
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
					CHECK_CLOSE(pow(f,3), m2[row * 4 + col], precision);
					f += 1.f;
				}
				else {
					CHECK_CLOSE(0.f, m2[row * 4 + col], precision);
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
			CHECK_CLOSE(m1[i], m2[i], precision);
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
			CHECK_CLOSE(m1[i], m2[i], precision);
		}*/
	}

	TEST(lup) {
		Matrix3x3 A = {
			9.f,	6.f,	0.f,	
			6.f,	5.f,	4.f,	
			3.f,	4.f,	10.f};
		Matrix3x3 L, U, P;
		A.LUP(&L, &U, &P);
		Matrix3x3 PA = P * A;
		Matrix3x3 LU = L * U;
		for (int i = 0; i < 9; i++) {
			CHECK_CLOSE(PA[i], LU[i], precision);
		}
	}

	TEST(inverse) {
		Matrix3x3 A = {
			9.f,	6.f,	0.f,
			6.f,	5.f,	4.f,
			3.f,	4.f,	10.f };
		Matrix3x3 invA = A.inverse();
		Matrix3x3 AinvA = A * invA;
		Matrix3x3 I = Matrix3x3::newIdentity();
		for (int i = 0; i < 9; i++) {
			CHECK_CLOSE(AinvA[i], I[i], precision);
		}
	}

	TEST(det) {
		Matrix3x3 A = {
			9.f,	6.f,	0.f,
			6.f,	5.f,	4.f,
			3.f,	4.f,	10.f };
		CHECK_CLOSE(18.0, A.det(), precision);
	}
}
