#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "base.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "light.hpp"

namespace renderer {
	class Shape;
	class PerspectiveCamera;
	class Film;
	class Light;
	class ShapeUnion;

	class SceneDesc {
	public:
		int threadsPow;
		int width;
		int height;
		int maxReflect;
		Film* film;
		PerspectiveCamera* camera;
		MaterialDict matDict;
		ShapeUnion* shapeUnion;
		Lights lights;
		SceneDesc() :
			threadsPow(0), width(1), height(1),
			maxReflect(0), film(nullptr), camera(nullptr),
			shapeUnion(nullptr) {}
	};

	class Renderer {
	public:
		void renderDepth(Film*, Shape&, PerspectiveCamera&, float maxDepth);
		void renderNormal(Film*, Shape& scene, PerspectiveCamera& camera, float maxDepth);
		void rayTrace(Film*, Shape& scene, PerspectiveCamera& camera, Lights&);
		Color rayTraceRecursive(Shape* scene, Ray& ray, Lights&, int maxReflect);
		void rayTraceReflection(Film*, Shape* scene, PerspectiveCamera& camera, Lights&, int maxReflect, int x = 0, int y = 0, int w = 0, int h = 0);
		void ConcurrentRender(SceneDesc&);

	};
}


#endif // RENDERER_RENDERER_HPP
