#include "stdafx.h"
#include "renderer.hpp"
#include "camera.hpp"
#include "shape.hpp"
#include "intersect_result.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "color.hpp"
#include "ray.hpp"
#include "film.hpp"

namespace renderer {

	void Renderer::renderDepth(Film *film, Shape& scene, PerspectiveCamera& camera, float maxDepth) {
		scene.Init();
		camera.Init();
		int w = film->width(), h = film->height();
		IntersectResult result;
		for (int y = 0; y < h; y++) {
			float sy = 1.0f - (float)y / h;
			for (int x = 0; x < w; x++) {
				float sx = (float)x / w;
				Ray& ray = camera.GenerateRay(sx, sy);
				scene.Intersect(ray, &result);
				if (result.geometry) {
					int depth = int(255.0f - min((result.distance / maxDepth) * 255.0f, 255.0f));
					film->set(x, y, depth, depth, depth);
				}
			}
		}
	}
	void Renderer::renderNormal(Film *film, Shape& scene, PerspectiveCamera& camera, float maxDepth) {
		scene.Init();
		camera.Init();
		int w = film->width(), h = film->height();
		IntersectResult result;
		for (int y = 0; y < h; y++) {
			float sy = 1.0f - (float)y / h;
			for (int x = 0; x < w; x++) {
				float sx = (float)x / w;
				Ray&& ray = camera.GenerateRay(sx, sy);
				scene.Intersect(ray, &result);
				if (result.geometry) {
					Vector3dF& pNormal = result.normal;
					film->set(x, y, 
						(pNormal.x + 1.0f) * 128.0f,
						(pNormal.y + 1.0f) * 128.0f,
						(pNormal.z + 1.0f) * 128.0f);
				}
			}
		}
	};


	void Renderer::rayTrace(Film *film, Shape& scene, PerspectiveCamera& camera) {
		scene.Init();
		camera.Init();
		int w = film->width(), h = film->height();
		IntersectResult result;
		for (int y = 0; y < h; y++) {
			float sy = 1.0f - (float)y / h;
			for (int x = 0; x < w; x++) {
				float sx = (float)x / w;
				Ray& ray = camera.GenerateRay(sx, sy);
				scene.Intersect(ray, &result);
				if (result.geometry) {
					Material* pMaterial = result.geometry->material;
					Color color = pMaterial->Sample(ray, result.position, result.normal);
					//printf("c=%f,%f,%f\n", color->r(),color->g(),color->b());
					film->set(x, y, 
						min(int(color.r() * 255), 255),
						min(int(color.g() * 255), 255),
						min(int(color.b() * 255), 255));
				}
			}
		}
	}

	Color Renderer::rayTraceRecursive(Shape* scene, Ray& ray, int maxReflect) {
		IntersectResult result;
		scene->Intersect(ray, &result);
		if (result.geometry) {
			Material* pMaterial = result.geometry->material;
			float reflectiveness = pMaterial->reflectiveness;
			Color color = pMaterial->Sample(ray, result.position, result.normal);
			color = Color(color * (1.0f - reflectiveness));

			if (reflectiveness > 0 && maxReflect > 0) {
				Vector3dF r = Vector3dF(result.normal * (-2 * (result.normal.Dot(ray.d))) + ray.d);
				Ray new_ray(result.position, r);
				Color reflectedColor = rayTraceRecursive(scene, new_ray, maxReflect - 1);
				assert(reflectedColor.r() >= 0 && reflectedColor.g() >= 0 && reflectedColor.b() >= 0);
				color = color + reflectedColor * reflectiveness;
			}
			return color;
		}
		else
			return Color::Black;
	}

	void Renderer::rayTraceReflection(Film *film, Shape* scene, PerspectiveCamera& camera, int maxReflect, int px, int py, int pw, int ph) {
		scene->Init();
		camera.Init();
		int w = pw, h = ph, img_width = film->width(), img_height = film->height();
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
				Color color = rayTraceRecursive(&(*scene), ray, maxReflect);
				film->set(x, y, 
					min(int(color.r() * 255), 255),
					min(int(color.g() * 255), 255),
					min(int(color.b() * 255), 255));
			}
		}
	}
}