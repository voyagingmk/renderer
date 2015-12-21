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
#include <type_traits>
#include <algorithm>

#include "stdafx.h"


#define DEFINE_SHARE_PTR(name) typedef std::shared_ptr<name> Ptr#name;
#define FLOAT_MAX (std::numeric_limits<float>::max())

#define DEBUG_RENDERER

#ifdef DEBUG_RENDERER
#define Assert(expr) \
    ((expr) ? (void)0 : \
        Severe("[Assert] \"%s\" failed in %s, line %d", \
               #expr, __FILE__, __LINE__))
#else
#define Assert(expr) ((void)0)
#endif

#define DEBUG_POINT(msg)
//#define DEBUG_POINT(msg) printf("%s [x, y, z = %f, %f, %f]\n", msg, x, y, z)

namespace renderer {

	using std::min;
	using std::max;

	class IntersectResult;
	class Shape;
	class Material;
	class Point;
	class Sphere;
	class Plane;
	class Union;
	class Color;
	class Ray;

	typedef Point Normal;
	typedef Point Vector;

	template<class T>
	static MemoryPool<T>* GetPool() {
		static MemoryPool<T> pool;
		return &pool;
	}

	constexpr float PI = 3.141592653589793238463f;
}

#endif // BASE_HPP
