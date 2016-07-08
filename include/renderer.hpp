#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "base.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "shapes\union.hpp"

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
		PerspectiveCamera camera;
		MaterialDict matDict;
		ShapeUnion shapeUnion;
		Lights lights;
		SceneDesc(Film* f, const PerspectiveCamera& c, ShapeUnion&& s) :
			threadsPow(0), width(1), height(1),
			maxReflect(0), film(f), camera(c),
			shapeUnion(std::forward<ShapeUnion>(s)) {
		}
		SceneDesc(SceneDesc&& s):
			threadsPow(s.threadsPow),
			width(s.width),
			height(s.height),
			maxReflect( s.maxReflect),
			film(s.film),
			camera(s.camera)
		{
			s.film = nullptr;
			matDict = std::move(s.matDict);
			lights = std::move(s.lights);
			shapeUnion = std::move(s.shapeUnion);
		}
		~SceneDesc() {
			film = nullptr;
		}
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
