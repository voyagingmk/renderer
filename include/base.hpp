#ifndef BASE_HPP
#define BASE_HPP

#if _MSC_VER

#else

#endif 
#include <algorithm>
#include <memory>
#include <SDL/SDL.h>
#include <GL/GL.h>
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

	typedef unsigned int uint;
	typedef double Real;

	class IntersectResult;
	class Shape;
	class Material;
	class Sphere;
	class Plane;
	class Union;
	class Color;
	class Ray;

	template<class T>
	static MemoryPool<T>* GetPool() {
		static MemoryPool<T> pool;
		return &pool;
	}

	constexpr float PI = 3.141592653589793238463f;

	template<class T>
	typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
		almost_equal(T x, T y, int ulp)
	{
		// the machine epsilon has to be scaled to the magnitude of the values used
		// and multiplied by the desired precision in ULPs (units in the last place)
		return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
			// unless the result is subnormal
			|| std::abs(x - y) < std::numeric_limits<T>::min();
	}
}

#endif // BASE_HPP
