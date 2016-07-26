#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "base.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "shapes\union.hpp"
#include <bitset>

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
		int threadsNum() {
			return std::pow(2, threadsPow);
		}
		void init();
	};

	class Renderer {
		int preCount;
		int curRow;
	public:
		int pRendered;   //rendered pixels 
		int pDispatched; //dispatched pixels, means renderring
		bool* flags;
		Color* colorArray;
		std::mutex mtx;
		std::vector<std::thread*> threads;
		int countRenderedPixels();
	public:
		Renderer(SceneDesc& desc):
			pRendered(0),
			pDispatched(0),
			preCount(0),
			curRow(0),
			flags(new bool[desc.width * desc.height]{ false }),
			colorArray(new Color[desc.width * desc.height]) {
			for (int i = 0; i < desc.width * desc.height; i++) {
			//	colorArray[i].rgb[0] ;
			}
		}
		~Renderer() {
			for (int i = 0; i < threads.size(); i++) {
				delete threads[i];
			}
			if (flags)
				delete[] flags;
			if (colorArray)
				delete[] colorArray;

		}
		void renderDepth(Film*, Shape&, PerspectiveCamera&, float maxDepth);
		void renderNormal(Film*, Shape& scene, PerspectiveCamera& camera, float maxDepth);
		void rayTrace(Film*, Shape& scene, PerspectiveCamera& camera, Lights&);
		Color rayTraceRecursive(Shape* scene, Ray& ray, Lights&, int maxReflect);
		void rayTraceReflection(Film*, Shape* scene, PerspectiveCamera& camera, Lights&, int maxReflect, int x = 0, int y = 0, int w = 0, int h = 0);
		void rayTraceConcurrence(SceneDesc&);
		void beginAsyncRender(SceneDesc& desc);
		void endAsyncRender();
		void getRenderRect(SceneDesc& desc, int* x, int* y, int* w, int* h);
		Color rayTraceAt(SceneDesc&, int x, int y);
		void renderScene(SceneDesc&);
	};
}


#endif // RENDERER_RENDERER_HPP
