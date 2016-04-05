#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class Shape;
	class PerspectiveCamera;
	class Film;
	class Light;

	class Renderer {
	public:
		void renderDepth(Film*, Shape&, PerspectiveCamera&, float maxDepth);
		void renderNormal(Film*, Shape& scene, PerspectiveCamera& camera, float maxDepth);
		void rayTrace(Film*, Shape& scene, PerspectiveCamera& camera, std::vector<Light*>&);
		Color rayTraceRecursive(Shape* scene, Ray& ray, std::vector<Light*>&, int maxReflect);
		void rayTraceReflection(Film*, Shape* scene, PerspectiveCamera& camera, std::vector<Light*>&, int maxReflect, int x = 0, int y = 0, int w = 0, int h = 0);
	};
}


#endif // RENDERER_RENDERER_HPP
