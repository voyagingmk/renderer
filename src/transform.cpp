#include "stdafx.h"
#include "transform.hpp"

namespace renderer {

	Transform Transform::operator*(const Transform &t2) const {
		return Transform(m * t2.m, t2.mInv * mInv);
	}
}
