#include "stdafx.h"
#include "renderer.hpp"
#include "camera.hpp"
#include "shape.hpp"
#include "intersect_result.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "color.hpp"
#include "ray.hpp"

namespace renderer {

	void Renderer::renderDepth(cil::CImg<unsigned char> &img, Shape& scene, PerspectiveCamera& camera, float maxDepth) {
		scene.Init();
		camera.Init();
		int w = img.width(), h = img.height();
		IntersectResult result;
		for (int y = 0; y < h; y++) {
			float sy = 1.0f - (float)y / h;
			for (int x = 0; x < w; x++) {
				float sx = (float)x / w;
				Ray& ray = camera.GenerateRay(sx, sy);
				scene.Intersect(ray, &result);
				if (result.geometry) {
					int depth = int(255.0f - min((result.distance / maxDepth) * 255.0f, 255.0f));
					img.atXYZC(x, y, 0, 0) = depth;
					img.atXYZC(x, y, 0, 1) = depth;
					img.atXYZC(x, y, 0, 2) = depth;
					//atXYZC(x, y, 0, 0) = 255;
				}
			}
		}
	}
	void Renderer::renderNormal(cil::CImg<unsigned char> &img, Shape& scene, PerspectiveCamera& camera, float maxDepth) {
		scene.Init();
		camera.Init();
		int w = img.width(), h = img.height();
		IntersectResult result;
		for (int y = 0; y < h; y++) {
			float sy = 1.0f - (float)y / h;
			for (int x = 0; x < w; x++) {
				float sx = (float)x / w;
				Ray&& ray = camera.GenerateRay(sx, sy);
				scene.Intersect(ray, &result);
				if (result.geometry) {
					Vector& pNormal = result.normal;
					img.atXYZC(x, y, 0, 0) = (pNormal.x + 1.0f) * 128.0f;
					img.atXYZC(x, y, 0, 1) = (pNormal.y + 1.0f) * 128.0f;
					img.atXYZC(x, y, 0, 2) = (pNormal.z + 1.0f) * 128.0f;
					//atXYZC(x, y, 0, 0) = 255;
				}
			}
		}
	};


	void Renderer::rayTrace(cil::CImg<unsigned char> &img, Shape& scene, PerspectiveCamera& camera) {
		scene.Init();
		camera.Init();
		int w = img.width(), h = img.height();
		IntersectResult result;
		for (int y = 0; y < h; y++) {
			float sy = 1.0f - (float)y / h;
			for (int x = 0; x < w; x++) {
				float sx = (float)x / w;
				Ray& ray = camera.GenerateRay(sx, sy);
				scene.Intersect(ray, &result);
				if (result.geometry) {
					PtrMaterial pMaterial = result.geometry->material;
					PtrColor color = pMaterial->Sample(ray, result.position, result.normal);
					//printf("c=%f,%f,%f\n", color->r(),color->g(),color->b());
					img.atXYZC(x, y, 0, 0) = min(int(color->r() * 255), 255);
					img.atXYZC(x, y, 0, 1) = min(int(color->g() * 255), 255);
					img.atXYZC(x, y, 0, 2) = min(int(color->b() * 255), 255);
				}
			}
		}
	}

	PtrColor Renderer::rayTraceRecursive(Shape* scene, Ray& ray, int maxReflect) {
		IntersectResult result;
		scene->Intersect(ray, &result);
		if (result.geometry) {
			PtrMaterial pMaterial = result.geometry->material;
			float reflectiveness = pMaterial->reflectiveness;
			PtrColor color = pMaterial->Sample(ray, result.position, result.normal);
			color = std::make_shared<Color>(*color * (1.0f - reflectiveness));

			if (reflectiveness > 0 && maxReflect > 0) {
				PtrVector r = std::make_shared<Vector>(result.normal * (-2 * (result.normal.Dot(ray.d))) + ray.d);
				Ray new_ray(result.position, *r);
				PtrColor reflectedColor = rayTraceRecursive(scene, new_ray, maxReflect - 1);
				assert(reflectedColor->r() >= 0 && reflectedColor->g() >= 0 && reflectedColor->b() >= 0);
				*color = *color + *reflectedColor * reflectiveness;
			}
			return color;
		}
		else
			return Color::Black;
	}

	void Renderer::rayTraceReflection(cil::CImg<unsigned char> &img, PtrShape scene, PerspectiveCamera& camera, int maxReflect, int px, int py, int pw, int ph) {
		scene->Init();
		camera.Init();
		int w = pw, h = ph, img_width = img.width(), img_height = img.height();
		if (w == 0)
			w = img_width;
		if (h == 0)
			h = img_height;
		for (int y = py, yMax = py + h; y < yMax; y++) {
			float sy = 1.0f - (float)y / img_height;
			for (int x = px, xMax = px + w; x < xMax; x++) {
				float sx = (float)x / img_width;
				//printf("sx,sy=%f,%f\n",sx,sy);
				Ray& ray = camera.GenerateRay(sx, sy);
				PtrColor color = rayTraceRecursive(&(*scene), ray, maxReflect);
				img.atXYZC(x, y, 0, 0) = min(int(color->r() * 255), 255);
				img.atXYZC(x, y, 0, 1) = min(int(color->g() * 255), 255);
				img.atXYZC(x, y, 0, 2) = min(int(color->b() * 255), 255);
			}
		}
	}
}