#include "stdafx.h"
#include "base.hpp"
#include "raytracer.hpp"
#include "camera.hpp"
#include "shape.hpp"
#include "intersect_result.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "color.hpp"
#include "ray.hpp"
#include "film.hpp"
#include "light.hpp"
#include "union.hpp"
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
		shapeUnion->Init();
		camera.Init();
	}

	void RayTracer::rayTrace(Film *film, Shape* scene, PerspectiveCamera& camera, Lights& lights) {
		int w = film->width(), h = film->height();
		IntersectResult result;
		for (int y = 0; y < h; y++) {
			float sy = 1.0f - (float)y / h;
			for (int x = 0; x < w; x++) {
				float sx = (float)x / w;
				Ray ray = camera.GenerateRay(sx, sy);
				scene->Intersect(ray, &result);
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

	void createCoordinateSystem(const Vector3dF &axisY, Vector3dF &axisZ, Vector3dF &axisX)
	{
		if (std::fabs(axisY.x) > std::fabs(axisY.y))
			axisZ = Vector3dF(axisY.z, 0, -axisY.x) / sqrtf(axisY.x * axisY.x + axisY.z * axisY.z);
		else
			axisZ = Vector3dF(0, -axisY.z, axisY.y) / sqrtf(axisY.y * axisY.y + axisY.z * axisY.z);
		axisZ = -axisZ; //trans to left handness
		axisX = axisY.Cross(axisZ);
		axisZ = axisY.Cross(axisX);//make it orthogonal
	}

	Vector3dF uniformSampleHemisphere(const float &r1, const float &r2)
	{
		// cos(theta) = r1 = y
		// cos^2(theta) + sin^2(theta) = 1 -> sin(theta) = srtf(1 - cos^2(theta))
		float sinTheta = sqrtf(1 - r1 * r1);
		float phi = 2 * M_PI * r2;
		float x = sinTheta * cosf(phi);
		float z = sinTheta * sinf(phi);
		return Vector3dF(x, r1, z);
	}

	std::mt19937 eng(4029349);
	Color RayTracer::rayTraceRecursive(Shape* scene, Ray& ray, Lights& lights, int maxReflect) {
		IntersectResult result;
		scene->Intersect(ray, &result);
		if (!result.geometry) {
			logDebug("rayTraceRecursive, no geometry\n\n");
			return Color::Black;
		}
		logDebug("rayTraceRecursive, hit geometry\n\n");
		Material* pMaterial = result.geometry->material;
		float reflectiveness = pMaterial->reflectiveness;
		Color color(0, 0, 0);
		
		std::uniform_real_distribution<float> distribution(0, 1);
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
				c = pMaterial->Sample(ray, result.position, result.normal, incidenceNormal);
				float disToLight = 0;
				if (pLight->lightType == LightType_Point) {
					disToLight = (dynamic_cast<PointLight*>(pLight)->pos - result.position).Length();
				}
				int N = pLight->shadowrays;
				int hitTimes = 0;
				Vector3dF axisZ, axisX;
				Vector3dF& axisY = incidenceNormal;
				createCoordinateSystem(axisY, axisZ, axisX);
				float lightRadius = (dynamic_cast<PointLight*>(pLight))->radius;
				for (int i = 0; i < N; i++) {
					float r1 = distribution(eng);
					float r2 = distribution(eng);
					Vector3dF sample = uniformSampleHemisphere(r1, r2);
					Vector3dF sampleWorld(
						sample.x * axisX.x + sample.y * axisY.x + sample.z * axisZ.x,
						sample.x * axisX.y + sample.y * axisY.y + sample.z * axisZ.y,
						sample.x * axisX.z + sample.y * axisY.z + sample.z * axisZ.z);
					Ray shadowrays(result.position, (-incidenceCenter) + sampleWorld * lightRadius);
					shadowrays.d = shadowrays.d.Normalize();
					IntersectResult _result;
					scene->Intersect(shadowrays, &_result);
					if (_result.geometry && _result.geometry != result.geometry) {
						if (disToLight && _result.tHit >= disToLight) {
							continue;
						}
						hitTimes++;
					}
					//printf("\n");
				}
				logDebug("hitTimes:%d\n", hitTimes);
				if (hitTimes > 0) {
					//printf("%d\n", hitTimes);
					float black_ratio = (float)hitTimes / (float)N;
					// c = c * ( 1.0f - black_ratio) + Color::Black * black_ratio;
					// c = c.Modulate(Color::White * (1.0f - black_ratio));
					// c = c.clamp();
					c *= 1.0f - black_ratio;
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
				if (_result.geometry && _result.geometry != result.geometry) {
					logDebug("shadowed, id1: %d id2: %d \n", _result.geometry->id, result.geometry->id);
					if (pLight->lightType == LightType_Point) {
						float disToLight = (dynamic_cast<PointLight*>(pLight)->pos - result.position).Length();
						if (disToLight >= _result.tHit) {
							canSample = false;
							c = Color::Black;
						}
					}
					else if (pLight->lightType == LightType_Direction) {
						canSample = false;
						c = Color::Black;
					}
				}
				logDebug("canSample %d", int(canSample));
				if(canSample){
					c = pMaterial->Sample(ray, result.position, result.normal, incidenceNormal);
				}
			}
			color = color + c;
		}

		color = color * (1.0f - reflectiveness);
		if (Enable_IndirectDiffuse && maxReflect == sceneDesc->maxReflect) {
			Color indirectDiffuse;
			Vector3dF axisZ, axisX;
			Normal3dF& axisY = result.normal;
			createCoordinateSystem(axisY, axisZ, axisX);
			for (uint32_t n = 0; n < SampleNum_IndirectDiffuse; ++n) {
				float r1 = distribution(eng); // cos(theta) = N.Light Direction 
				float r2 = distribution(eng);
				Vector3dF sample = uniformSampleHemisphere(r1, r2);
				//    axisX
				//M = axisY
				//	  axisZ
				//vector_local * M = vector_world
				Vector3dF sampleWorld(
					sample.x * axisX.x + sample.y *  axisY.x + sample.z * axisZ.x,
					sample.x * axisX.y + sample.y *  axisY.y + sample.z * axisZ.y,
					sample.x * axisX.z + sample.y *  axisY.z + sample.z * axisZ.z);
				Ray indirect_ray(result.position, sampleWorld);
				indirectDiffuse += rayTraceRecursive(scene, indirect_ray, lights, 0) * r1;
			}
			// divide by N and the constant PDF
			indirectDiffuse = indirectDiffuse / (float(SampleNum_IndirectDiffuse) * (1.0 / (2.0 * M_PI)));
			color += indirectDiffuse / M_PI;
		}
			
		if (reflectiveness > 0 && maxReflect > 0) {
			Vector3dF d = Vector3dF(result.normal * (-2 * (result.normal.Dot(ray.d))) + ray.d);
			Ray new_ray(result.position, d);
			Color reflectedColor = rayTraceRecursive(scene, new_ray, lights, maxReflect - 1);
			assert(reflectedColor.r() >= 0 && reflectedColor.g() >= 0 && reflectedColor.b() >= 0);
			color += reflectedColor * reflectiveness;
		}
		return color;
	}

	void RayTracer::rayTraceReflection(Film *film, Shape* scene, PerspectiveCamera& camera, Lights& lights, int maxReflect, int px, int py, int pw, int ph) {
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
				Ray ray = camera.GenerateRay(sx, sy);
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

	Color RayTracer::rayTraceAt(SceneDesc& desc, int x, int y) {
		int w = 1, 
			h = 1,
			img_width = desc.film->width(),
			img_height = desc.film->height();
		float sy = 1.0f - (float)y / img_height;
		float sx = (float)x / img_width;
		Ray ray = desc.camera.GenerateRay(sx, sy);
		if (Enable_DebugPixcel) {
			printf("x,y = %d,%d, sx,sy = %.3f,%.3f   %d,%d\n", x,y, sx, sy, img_width, img_height);
		}
		switch (RenderType) {
		case RenderType_Default:
			return rayTraceRecursive(desc.shapeUnion, ray, desc.lights, desc.maxReflect);
			break;
		case RenderType_DepthMap:
			{
				IntersectResult result;
				desc.shapeUnion->Intersect(ray, &result);
				if (result.geometry) {
					float depth = min((result.tHit / 200.f), 1.0f);
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
				desc.shapeUnion->Intersect(ray, &result);
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
        default:
            break;
		}
        return Color();
    }

	void _rayTraceTask(RayTracer *renderer, SceneDesc& desc) 
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

	int RayTracer::countRenderedPixels()
	{
		std::lock_guard<std::mutex> lock(mtx);
		return pRendered;
	}

	int RayTracer::getRenderRect(SceneDesc& desc, int* x, int* y, int* w, int* h) {
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
		return newCount;
	}

	void RayTracer::beginAsyncRender(SceneDesc& desc)
	{
		int threadsNum = desc.threadsNum();
		threads.resize(threadsNum);
		for (int i = 0; i < threadsNum; i++) {
			threads[i] = new std::thread(_rayTraceTask, this, std::ref(desc));
		}
	}

	void RayTracer::endAsyncRender() {
		for (int i = 0; i < threads.size(); i++) {
			threads[i]->join();
		}
	}

	void renderArea(RayTracer *renderer, SceneDesc& desc, int x, int y, int w, int h)
	{
		renderer->rayTraceReflection(desc.film, desc.shapeUnion, desc.camera, desc.lights, desc.maxReflect, x, y, w, h);
	}

	void RayTracer::rayTraceConcurrence(SceneDesc& desc)
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

	void RayTracer::renderScene(SceneDesc& desc) {
		if (desc.threadsPow == 0) {
			rayTrace(desc.film, desc.shapeUnion, desc.camera, desc.lights);
			rayTraceReflection(desc.film, desc.shapeUnion, desc.camera, std::ref(desc.lights), 4);
		}
		else {
			rayTraceConcurrence(desc);
		}
	}
}
