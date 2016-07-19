#include "stdafx.h"
#include "transform.hpp"

namespace renderer {

	Transform4x4 Transform4x4::operator*(const Transform4x4 &t) const {
		return Transform4x4(m * t.m, t.mInv * mInv);
	}
}
