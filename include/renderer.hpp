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
		SceneDesc(const PerspectiveCamera& c, ShapeUnion&& s) :
			threadsPow(0), width(1), height(1),
			maxReflect(0), film(nullptr), camera(c),
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
		void setFilm(Film* f);
	};

	class Renderer {
	public:
		void renderDepth(Film*, Shape&, PerspectiveCamera&, float maxDepth);
		void renderNormal(Film*, Shape& scene, PerspectiveCamera& camera, float maxDepth);
		void rayTrace(Film*, Shape& scene, PerspectiveCamera& camera, Lights&);
		Color rayTraceRecursive(Shape* scene, Ray& ray, Lights&, int maxReflect);
		void rayTraceReflection(Film*, Shape* scene, PerspectiveCamera& camera, Lights&, int maxReflect, int x = 0, int y = 0, int w = 0, int h = 0);
		void rayTraceConcurrence(SceneDesc&);
		void initRenderDesc(SceneDesc&);
		void asyncRender(SceneDesc& desc, std::mutex& mtx, int p);
		Color rayTraceAt(SceneDesc&, int x, int y);
		void renderScene(SceneDesc&);
	};
}


#endif // RENDERER_RENDERER_HPP
