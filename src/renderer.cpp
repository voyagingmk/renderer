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
#include "profiler.hpp"
#include "defines.hpp"

namespace renderer {

	void SceneDesc::setFilm(Film* f) {
		film = f;
		if (film->width() != width || film->height() != height)
			film->resize(width, height);
	}

	void SceneDesc::init() {
		//no more than 16 threads
		if (threadsPow > 4)
			threadsPow = 4;
		int num = threadsNum();
		//make sure width is times of threadsNum
		if (num != 0 && width / num) {
			width -= width % num;
		}
		film->resize(width, height);
		shapeUnion.Init();
		camera.Init();
	}

	void Renderer::rayTrace(Film *film, Shape& scene, PerspectiveCamera& camera, Lights& lights) {
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
				Light* pLight = lights[i];
				Color c;
				if (!pLight->shadow) {
					//no shadow
					Vector3dF incidenceNormal = pLight->incidenceNormal(result.position);
					c = pMaterial->Sample(ray, result.position, result.normal, incidenceNormal);
				} else if (pLight->softshadow) {
					Vector3dF incidenceCenter = pLight->incidence(result.position);
					Vector3dF incidenceNormal = incidenceCenter.Normalize();
					Vector3dF rayNormal(-incidenceCenter.y, incidenceCenter.x, 0);
					rayNormal = rayNormal.Normalize();
					float disToLight = 0;
					if (pLight->lightType == LightType_Point) {
						disToLight = (dynamic_cast<PointLight*>(pLight)->pos - result.position).Length();
					}
					int hitTimes = 0;
					int raysPerFan = pLight->shadowrays / 4;
					for (int quadrant = 0; quadrant < 4; quadrant++) {
						for (int r = 0; r < raysPerFan; r++) {
							float angle = quadrant * 90.0f + fraction_dist(eng) * 90.f;
							float dis = fraction_dist(eng) * pLight->radius;
							//printf("<%.1f, %.1f> ", angle, dis);
							Vector3dF d = rayNormal.rotate(incidenceNormal, PI * angle / 180.f);
							Ray shadowrays(result.position, (-incidenceCenter) + d * dis);
							shadowrays.d = shadowrays.d.Normalize();
							IntersectResult _result;
							scene->Intersect(shadowrays, &_result);
							if (_result.geometry) {
								if (disToLight && _result.distance >= disToLight) {
									continue;
								}
								hitTimes++;
							}
						}
						//printf("\n");
					}
					//printf("\n");
					c = pMaterial->Sample(ray, result.position, result.normal, incidenceNormal);
					if (hitTimes > 0) {
						//printf("%d\n", hitTimes);
						float black_ratio = hitTimes / (float)pLight->shadowrays;
						//c = c * ( 1.0f - black_ratio) + Color::Black * black_ratio;
						c = c.Modulate(Color::White * (1.0f - black_ratio));
						c = c.clamp();
					}
				}
				else {
					//normal shadow
					Vector3dF incidenceNormal = pLight->incidenceNormal(result.position);
					//Is this light visible 
					Ray shadowrays(result.position, -incidenceNormal);
					IntersectResult _result;
					scene->Intersect(shadowrays, &_result);
					bool canSample = true;
					if (_result.geometry) {
						if (pLight->lightType == LightType_Point) {
							float disToLight = (dynamic_cast<PointLight*>(pLight)->pos - result.position).Length();
							if (disToLight >= _result.distance) {
								canSample = false;
								c = Color::Black;
							}
						}
						else if (pLight->lightType == LightType_Direction) {
							canSample = false;
							c = Color::Black;
						}
					}
					if(canSample){
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

	Color Renderer::rayTraceAt(SceneDesc& desc, int x, int y) {
		int w = 1, 
			h = 1,
			img_width = desc.film->width(),
			img_height = desc.film->height();
		float sy = 1.0f - (float)y / img_height;
		float sx = (float)x / img_width;
		Ray& ray = desc.camera.GenerateRay(sx, sy);
		//printf("x,y = %d,%d, sx,sy = %.3f,%.3f   %d,%d\n", x,y, sx, sy, img_width, img_height);
		switch (RenderType) {
		case RenderType_Default:
			return rayTraceRecursive(&desc.shapeUnion, ray, desc.lights, desc.maxReflect);
			break;
		case RenderType_DepthMap:
			{
				IntersectResult result;
				desc.shapeUnion.Intersect(ray, &result);
				if (result.geometry) {
					float depth = min((result.distance / 200.f), 1.0f);
					return Color(depth, depth, depth).clamp();
				}
				else {
					return Color::Black;
				}
				break;
			}
		case RenderType_NormalMap: 
			{
				IntersectResult result;
				desc.shapeUnion.Intersect(ray, &result);
				if (result.geometry) {
					Normal3dF& n = result.normal;
					return Color(
						(n.x + 1.0f) * 0.5f,
						(n.y + 1.0f) * 0.5f,
						(n.z + 1.0f) * 0.5f);
				}
				else {
					return Color::Black;
				}
				break;
			}
		}
	}

	void _rayTraceTask(Renderer *renderer, SceneDesc& desc) 
	{
		int total = desc.width * desc.height;
		const int count = 16;
		int p = 0;
		while (1) {
			{
				//get jobs
				std::lock_guard<std::mutex> lock(renderer->mtx);
				if (renderer->pDispatched >= total) {
					//all jobs done
					break;
				}
				p = renderer->pDispatched;
				renderer->pDispatched += count;
				if (renderer->pDispatched >= total) {
					renderer->pDispatched = total;
				}
			}
			for (int i = 0; i < count; i++) {
				int _p = p + i;
				if (_p + 1 >= total)
					break;
				int x = _p % desc.width, y = _p / desc.width;
				renderer->colorArray[_p] = renderer->rayTraceAt(desc, x, y);
				renderer->flags[_p] = true;
			}
			{
				std::lock_guard<std::mutex> lock(renderer->mtx);
				if (renderer->pRendered >= renderer->pDispatched)
					continue;
				int finished_count = 0;
				for (int _p = renderer->pRendered; _p < renderer->pDispatched; _p++) {
					if (!renderer->flags[_p]) {
						break;
					}
					else {
						finished_count++;
					}
				}
				if (finished_count > 0)
					renderer->pRendered += finished_count;
			}
		}
	}

	int Renderer::countRenderedPixels()
	{
		std::lock_guard<std::mutex> lock(mtx);
		return pRendered;
	}

	void Renderer::getRenderRect(SceneDesc& desc, int* x, int* y, int* w, int* h) {
		int newCount = countRenderedPixels();
		bool move = false;
		if (curRow * desc.width < newCount - desc.width) {
			if (curRow + 1 < desc.height)
				move = true;
		}
		*x = 0;
		*y = curRow;
		*w = desc.width;
		*h = 1;
		if (curRow != desc.height - 1 && !move) {
			*x = preCount % desc.width;
			*w = std::max(1, std::min(*w - *x, newCount - preCount));
		}		
		if (move)
			curRow++;
		preCount = newCount;
	}

	void Renderer::beginAsyncRender(SceneDesc& desc)
	{
		int threadsNum = desc.threadsNum();
		threads.resize(threadsNum);
		for (int i = 0; i < threadsNum; i++) {
			threads[i] = new std::thread(_rayTraceTask, this, std::ref(desc));
		}
	}

	void Renderer::endAsyncRender() {
		for (int i = 0; i < threads.size(); i++) {
			threads[i]->join();
		}
	}

	void renderArea(Renderer *renderer, SceneDesc& desc, int x, int y, int w, int h)
	{
		renderer->rayTraceReflection(desc.film, dynamic_cast<Shape*>(&desc.shapeUnion), desc.camera, desc.lights, desc.maxReflect, x, y, w, h);
	}

	void Renderer::rayTraceConcurrence(SceneDesc& desc)
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

	void Renderer::renderScene(SceneDesc& desc) {
		if (desc.threadsPow == 0) {
			rayTrace(desc.film, desc.shapeUnion, desc.camera, desc.lights);
			rayTraceReflection(desc.film, &desc.shapeUnion, desc.camera, std::ref(desc.lights), 4);
		}
		else {
			rayTraceConcurrence(desc);
		}
		//parserObj(config["obj"]);
	}
}