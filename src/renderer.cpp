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
#include "light.hpp"
#include "shapes/union.hpp"

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


	void Renderer::rayTrace(Film *film, Shape& scene, PerspectiveCamera& camera, Lights& lights) {
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
					Color color(0, 0, 0);
					for (int i = 0; i < lights.size(); i++) {
						Vector3dF incidence = lights[i]->incidence(result.position);
						Color c = pMaterial->Sample(ray, result.position, result.normal, incidence);
						color = color + c;
					}
					//printf("c=%f,%f,%f\n", color->r(),color->g(),color->b());
					film->set(x, y, 
						min(int(color.r() * 255), 255),
						min(int(color.g() * 255), 255),
						min(int(color.b() * 255), 255));
				}
			}
		}
	}

	Color Renderer::rayTraceRecursive(Shape* scene, Ray& ray, Lights& lights, int maxReflect) {
		IntersectResult result;
		scene->Intersect(ray, &result);
		if (result.geometry) {
			Material* pMaterial = result.geometry->material;
			float reflectiveness = pMaterial->reflectiveness;
			Color color(0, 0, 0);
			std::mt19937 eng(4029349);
			std::uniform_real_distribution<float> fraction_dist;
			for (int i = 0; i < lights.size(); i++) {
				Color c;
				if (!lights[i]->shadow) {
					Vector3dF incidenceNormal = lights[i]->incidenceNormal(result.position);
					c = pMaterial->Sample(ray, result.position, result.normal, incidenceNormal);
				} else if (lights[i]->softshadow) {
					Vector3dF incidenceCenter = lights[i]->incidence(result.position);
					Vector3dF incidenceNormal = incidenceCenter.Normalize();
					Vector3dF rayNormal(-incidenceCenter.y, incidenceCenter.x, 0);
					rayNormal = rayNormal.Normalize();
					int hitTimes = 0;
					int raysPerFan = lights[i]->shadowrays / 4;
					for (int quadrant = 0; quadrant < 4; quadrant++) {
						for (int r = 0; r < raysPerFan; r++) {
							float angle = quadrant * 90.0f + fraction_dist(eng) * 90.f;
							float dis = fraction_dist(eng) * lights[i]->radius;
							//printf("<%.1f, %.1f> ", angle, dis);
							Vector3dF d = rayNormal.rotate(incidenceNormal, PI * angle / 180.f);
							Ray shadowrays(result.position, (-incidenceCenter) + d * dis);
							shadowrays.d = shadowrays.d.Normalize();
							IntersectResult _result;
							scene->Intersect(shadowrays, &_result);
							if (_result.geometry) {
								hitTimes++;
							}
						}
						//printf("\n");
					}
					//printf("\n");
					c = pMaterial->Sample(ray, result.position, result.normal, incidenceNormal);
					if (hitTimes > 0) {
						//printf("%d\n", hitTimes);
						float black_ratio = hitTimes / (float)lights[i]->shadowrays;
						//c = c * ( 1.0f - black_ratio) + Color::Black * black_ratio;
						c = c.Modulate(Color::White * (1.0f - black_ratio));
						c = c.clamp();
					}
				}
				else {
					Vector3dF incidenceNormal = lights[i]->incidenceNormal(result.position);
					//Is this light visible 
					Ray shadowrays(result.position, -incidenceNormal);
					IntersectResult _result;
					scene->Intersect(shadowrays, &_result);
					if (_result.geometry) {
						c = Color::Black;
					}
					else {
						c = pMaterial->Sample(ray, result.position, result.normal, incidenceNormal);
					}
				}
				color = color + c;
			}
			color = Color(color * (1.0f - reflectiveness));

			if (reflectiveness > 0 && maxReflect > 0) {
				Vector3dF r = Vector3dF(result.normal * (-2 * (result.normal.Dot(ray.d))) + ray.d);
				Ray new_ray(result.position, r);
				Color reflectedColor = rayTraceRecursive(scene, new_ray, lights, maxReflect - 1);
				assert(reflectedColor.r() >= 0 && reflectedColor.g() >= 0 && reflectedColor.b() >= 0);
				color = color + reflectedColor * reflectiveness;
			}
			return color;
		}
		else
			return Color::Black;
	}

	void Renderer::rayTraceReflection(Film *film, Shape* scene, PerspectiveCamera& camera, Lights& lights, int maxReflect, int px, int py, int pw, int ph) {
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
				Color color = rayTraceRecursive(&(*scene), ray, lights, maxReflect);
				int r = min(int(color.r() * 255), 255),
					g = min(int(color.g() * 255), 255),
					b = min(int(color.b() * 255), 255);
				//printf("[rgb] %d %d = %d %d %d\n", x, y, r, g, b);
				film->set(x, y, 
					r, g, b);
			}
		}
	}
	
	void renderArea(Renderer *renderer, SceneDesc& desc, int x, int y, int w, int h)
	{
		renderer->rayTraceReflection(desc.film, dynamic_cast<Shape*>(&desc.shapeUnion), desc.camera, desc.lights, desc.maxReflect, x, y, w, h);
	}

	void Renderer::ConcurrentRender(SceneDesc& desc)
	{
		int threads_num = int(pow(2.0, desc.threadsPow));
		std::thread* *threads = new std::thread*[threads_num];
		int h = desc.height / threads_num, h_left = desc.height % threads_num;
		for (int i = 0; i < threads_num; i++) {
			int start_h = i * h, len_h = h;
			if (i == threads_num - 1)
				len_h += h_left;
			std::thread* t = new std::thread(renderArea, 
				this, std::ref(desc),
				0, start_h, 
				desc.width, len_h);
			threads[i] = t;
		}
		for (int i = 0; i < threads_num; i++) {
			threads[i]->join();
		}
	}
}