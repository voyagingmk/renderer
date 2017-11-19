#include "stdafx.h"
#include "com/matrix.hpp"

namespace renderer {
	
// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/orthographic-projection-matrix
	Matrix4x4 Ortho(
		float left, float right,
		float bottom, float top,
		float zNear, float zFar
	)
	{
		const float A = 2.0f / (right - left);
		const float B = 2.0f / (top - bottom);

//#define DEPTH_ZERO_TO_ONE 1

#ifdef DEPTH_ZERO_TO_ONE
		const float C = -1.0f / (zFar - zNear);
#else
		const float C = -2.0f / (zFar - zNear);
#endif
		const float D = -(right + left) / (right - left);
		const float E = -(top + bottom) / (top - bottom);
#ifdef DEPTH_ZERO_TO_ONE
		const float F = -zNear / (zFar - zNear);
#else
		const float F = -(zFar + zNear) / (zFar - zNear);
#endif

		Matrix4x4 result{
			A,      0.0f,   0.0f,   D,
			0.0f,   B,      0.0f,   E,
			0.0f,   0.0f,   C,      F,
			0.0f,   0.0f,   0.0f,   1.0f
		};
		return result;
	}
}
