#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "base.hpp"
namespace renderer {
	class Shape;
	class PerspectiveCamera;

	class Renderer {
	public:
		void renderDepth(cil::CImg<unsigned char>&, Shape&, PerspectiveCamera&, float maxDepth);
		void renderNormal(cil::CImg<unsigned char> &img, Shape& scene, PerspectiveCamera& camera, float maxDepth);
		void rayTrace(cil::CImg<unsigned char> &img, Shape& scene, PerspectiveCamera& camera);
		PtrColor rayTraceRecursive(PtrGeometry scene, PtrRay ray, int maxReflect);
		void rayTraceReflection(cil::CImg<unsigned char> &img, PtrGeometry scene, PerspectiveCamera& camera, int maxReflect, int x = 0, int y = 0, int w = 0, int h = 0);
	};
}


#endif // RENDERER_HPP
