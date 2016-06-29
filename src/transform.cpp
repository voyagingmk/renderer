#include "stdafx.h"
#include "transform.hpp"

namespace renderer {

	Transform4x4 Transform4x4::operator*(const Transform4x4 &t2) const {
		return Transform4x4(m * t2.m, t2.mInv * mInv);
	}
}
