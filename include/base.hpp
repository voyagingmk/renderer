#pragma once  

#ifndef BASE_HPP
#define BASE_HPP


#include <functional>
#include <stdio.h>
#include <string>
#include <cmath>
#include <limits>
#include <vector>
#include <list>
#include <assert.h>
#include <type_traits>
#include <algorithm>
#include <fstream>
#include <map>
#include <thread>
#include <mutex>
#include <random>
#include <bitset>
#include <ctime>
#include <unordered_map>
#include <chrono>
#include "stdafx.h"

#ifdef far
#undef far
#endif

#ifdef near
#undef near
#endif

#define FLOAT_MAX (std::numeric_limits<float>::max())

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
	class MaterialSetting;
	class Sphere;
	class Plane;
	class Union;
	class Color;
	class Ray;

	enum class Axis: uint8_t {
		x = 0,
		y = 1,
		z = 2
	};

	template<class T>
	static MemoryPool<T>* GetPool() {
		static MemoryPool<T> pool;
		return &pool;
	}
    
    static size_t getTimeMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    static size_t getTimeS() {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

	constexpr float PI = 3.141592653589793238463f;

	//compare float
	template<class T>
	typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
		almost_equal(T x, T y, int ulp = 2)
	{
		// the machine epsilon has to be scaled to the magnitude of the values used
		// and multiplied by the desired precision in ULPs (units in the last place)
		return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
			// unless the result is subnormal
			|| std::abs(x - y) < std::numeric_limits<T>::min();
	}
    
    template<class T>
    bool AlmostEqual(T v1, T v2)
    {
        return std::fabs(v1 - v2) < T(0.0001);
    }
    
	template<typename T>
	T** new2DArray(int h, int w) {
		T ** v = new T*[h];
		for (int i = 0; i < h; i++) {
			v[i] = new T[w]{ 0 };
		}
		return v;
	}

	template<typename T>
	void del2DArray(T** v, int h) {
		for (int i = 0; i < h; i++) {
			delete[] v[i];
		}
		delete[] v;
	}

	static std::string readFile(const char * filepath) {
		std::ifstream f(filepath);
		std::string s((std::istreambuf_iterator<char>(f)),
			(std::istreambuf_iterator<char>()));
		f.close();
		return s;
	}

	static nlohmann::json readJson(const char * filepath) {
		using json = nlohmann::json;
		const std::string& s = readFile(filepath);
        if(s.size() <= 0) {
            printf("[readJson] no such file: %s\n", filepath);
            return json();
        }
		json j = json::parse(s);
		return j;
	}
	static float randomFloat(){
		static std::mt19937 eng(time(0));
		static std::uniform_real_distribution<float> fraction_dist(0.0f, 1.0f);
		return fraction_dist(eng);
	}

	inline float GammaCorrect(float value) {
		if (value <= 0.0031308f) return 12.92f * value;
		return 1.055f * std::pow(value, (float)(1.f / 2.4f)) - 0.055f;
	}

	template <typename T>
	inline T Clamp(T val, T low, T high) {
		if (val < low)
			return low;
		else if (val > high)
			return high;
		else
			return val;
	}

	template <class T, class Q>
	inline T Lerp(T a, T b, Q t)
	{
		return (T)(a * (1 - t) + b * t);
	}

	inline float Radians(float degree) {
		return ((float)M_PI / 180.f) * degree;
	}

	inline float Degrees(float radian) {
		return (180.f / (float)M_PI) * radian;
	}
    template<typename T>
    class Radian;
    template<typename T>
    class Degree;
    
    template<typename T>
    class Radian {
        public:
        Radian():
            radian((T)0.0)
        {}
        explicit Radian(T radian):
            radian(radian)
        {}
        Degree<T> ToDegree() { return Degree<T>(((T)180.0 / (T)M_PI ) * radian); }
        T radian;
    };
    
    template<typename T>
    class Degree {
    public:
        Degree():
            degree((T)0.0)
        {}
        explicit Degree(T degree):
            degree(degree)
        {}
        Radian<T> ToRadian() { return Radian<T>(((T)M_PI / (T)180.0) * degree); }
        T degree;
    };
    
    typedef Radian<float> RadianF;
    typedef Degree<float> DegreeF;
    
    static std::string ReplaceString(std::string subject, const std::string& search,
                              const std::string& replace) {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return subject;
    }
    
#ifdef _SDL_H

	static void checkSDLError(int line = -1)
	{
#ifndef NDEBUG
		const char *error = SDL_GetError();
		if (*error != '\0')
		{
			printf("SDL Error: %s\n", error);
			if (line != -1)
				printf(" + line: %i\n", line);
			SDL_ClearError();
		}
#endif
	}

#endif
}

#endif // BASE_HPP
