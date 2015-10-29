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

using std::min;
using std::max;

#define DEFINE_SHARE_PTR(name) typedef std::shared_ptr<name> Ptr#name;
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
typedef std::shared_ptr<Shape> PtrGeometry;
typedef std::shared_ptr<Material> PtrMaterial;
typedef std::shared_ptr<Vector> PtrVector;
typedef std::shared_ptr<Sphere> PtrSphere;
typedef std::shared_ptr<Plane> PtrPlane;
typedef std::shared_ptr<Union> PtrUnion;
typedef std::shared_ptr<Color> PtrColor;
typedef std::shared_ptr<Ray> PtrRay;

constexpr float PI  =3.141592653589793238463f;

#endif // BASE_HPP
