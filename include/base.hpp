#ifndef BASE_HPP
#define BASE_HPP

#if _MSC_VER

#else

#endif 
#include <algorithm>
#include <memory>
#include <stdio.h>
#include <string>
#include <math.h>
#include <limits>
#include <vector>
#include <list>
#include <assert.h>
#include "CImg.h"

#define DEFINE_SHARE_PTR(name) typedef std::shared_ptr<name> Ptr#name;

namespace renderer {

	using std::min;
	using std::max;

	class IntersectResult;
	class Shape;
	class Material;
	class Vector;
	class Sphere;
	class Plane;
	class Union;
	class Color;
	class Ray;

	typedef std::shared_ptr<IntersectResult> PtrIntersectResult;
	typedef std::shared_ptr<Shape> PtShape;
	typedef std::shared_ptr<Material> PtrMaterial;
	typedef std::shared_ptr<Vector> PtrVector;
	typedef std::shared_ptr<Sphere> PtrSphere;
	typedef std::shared_ptr<Plane> PtrPlane;
	typedef std::shared_ptr<Union> PtrUnion;
	typedef std::shared_ptr<Color> PtrColor;
	typedef std::shared_ptr<Ray> PtrRay;

	constexpr float PI = 3.141592653589793238463f;
}

#endif // BASE_HPP
