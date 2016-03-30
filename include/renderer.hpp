#ifndef RENDERER_RENDERER_HPP
#define RENDERER_RENDERER_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
	class Shape;
	class PerspectiveCamera;
	class Film;

	class Renderer {
	public:
		void renderDepth(Film*, Shape&, PerspectiveCamera&, float maxDepth);
		void renderNormal(Film*, Shape& scene, PerspectiveCamera& camera, float maxDepth);
		void rayTrace(Film*, Shape& scene, PerspectiveCamera& camera, Vector3dF& lightDir);
		Color rayTraceRecursive(Shape* scene, Ray& ray, Vector3dF& lightDir, int maxReflect);
		void rayTraceReflection(Film*, Shape* scene, PerspectiveCamera& camera, Vector3dF& lightDir, int maxReflect, int x = 0, int y = 0, int w = 0, int h = 0);
	};
}


#endif // RENDERER_RENDERER_HPP
