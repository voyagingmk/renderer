#include <fstream>
#include <map>
#include <stdio.h>
#include <thread>
#include <ctime>
#include <chrono>
#include "parser.hpp"
#include "json.hpp"
#include "geometry.hpp"
#include "renderer.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "color.hpp"
#include "plane.hpp"
#include "union.hpp"

namespace renderer {

	void renderArea(Renderer &renderer, cil::CImg<unsigned char> &img, PtrShape pUnion, PerspectiveCamera& camera, int maxReflect, int x, int y, int w, int h)
	{
		renderer.rayTraceReflection(img, std::static_pointer_cast<Shape>(pUnion), camera, maxReflect, x, y, w, h);
	}

	int Parser::parseFromFile(std::string path) {
		using json = nlohmann::json;
		std::ifstream f("config.json");
		std::string config_string((std::istreambuf_iterator<char>(f)),
			(std::istreambuf_iterator<char>()));
		f.close();
		json config = json::parse(config_string);

		int width = config["width"], height = config["height"], multithread = config["multithread"];
		printf("width: %d, height: %d, multithread: %d \n", width, height, multithread);

		cil::CImg<unsigned char> img(width, height, 1, 3);

		std::map<int, PtrMaterial> matDict;

		for (auto objinfo : config["material"]) {
			PtrMaterial pm = nullptr;
			if (objinfo["type"] == "Phone") {
				pm = std::make_shared<PhongMaterial>(parseColor(objinfo["diffuse"]),
					parseColor(objinfo["specular"]),
					objinfo["shininess"],
					objinfo["reflectiveness"]);
			}
			else if (objinfo["type"] == "Checker") {
				pm = std::make_shared<CheckerMaterial>(objinfo["scale"], objinfo["reflectiveness"]);
			}
			matDict[objinfo["id"]] = pm;
		}
		std::vector<PtrShape> vecGeo;
		for (auto objinfo : config["scene"]) {
			PtrShape pg = nullptr;
			if (objinfo["type"] == "Sphere") {
				auto pos = objinfo["pos"];
				pg = std::make_shared<Sphere>(
					Vector(pos[0], pos[1], pos[2]),
					objinfo["radius"]);
				pg->material = matDict[objinfo["matId"]];
			}
			else if (objinfo["type"] == "Plane") {
				auto normal = objinfo["normal"];
				pg = std::make_shared<Plane>(
					Vector(normal[0], normal[1], normal[2]),
					objinfo["distance"]);
				pg->material = matDict[objinfo["matId"]];
			}
			if (!pg)
				exit(1);
			vecGeo.push_back(pg);
		}
		PtrUnion pUnion = std::make_shared<Union>(vecGeo);

		auto eye = config["camera"]["eye"];
		auto front = config["camera"]["front"];
		auto up = config["camera"]["up"];

		PerspectiveCamera camera(Vector(eye[0], eye[1], eye[2]),
			Vector(front[0], front[1], front[2]),
			Vector(up[0], up[1], up[2]),
			config["camera"]["fov"]);

		Renderer renderer;

		auto time0 = std::chrono::system_clock::now();
		if (!multithread) {
			renderer.rayTrace(img, *pUnion, camera);
			renderer.rayTraceReflection(img, std::static_pointer_cast<Shape>(pUnion), camera, 4);
		}
		else {
			if (multithread == 1) {
				std::thread t1(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 0, width, height / 2);
				std::thread t2(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, height / 2, width, height / 2);
				t1.join();
				t2.join();
			}
			if (multithread == 2) {
				std::thread t1(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 0, width, height / 4);
				std::thread t2(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 1 * height / 4, width, height / 4);
				std::thread t3(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 2 * height / 4, width, height / 4);
				std::thread t4(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 3 * height / 4, width, height / 4);
				t1.join();
				t2.join();
				t3.join();
				t4.join();
			}
			if (multithread == 3) {
				std::thread t1(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 0, width, height / 8);
				std::thread t2(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 1 * height / 8, width, height / 8);
				std::thread t3(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 2 * height / 8, width, height / 8);
				std::thread t4(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 3 * height / 8, width, height / 8);
				std::thread t5(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 4 * height / 8, width, height / 8);
				std::thread t6(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 5 * height / 8, width, height / 8);
				std::thread t7(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 6 * height / 8, width, height / 8);
				std::thread t8(renderArea, std::ref(renderer), std::ref(img), std::static_pointer_cast<Shape>(pUnion), std::ref(camera), 4, 0, 7 * height / 8, width, height / 8);
				t1.join();
				t2.join();
				t3.join();
				t4.join();
				t5.join();
				t6.join();
				t7.join();
				t8.join();
			}
		}
		auto time1 = std::chrono::system_clock::now();
		auto time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
		printf("cost: %lldms\n", time_cost);
		img.display("");
		return 0;
	}

	PtrColor Parser::parseColor(std::string c) {
		if (c == "Red")
			return Color::Red;
		else if (c == "White")
			return Color::White;
		else if (c == "Black")
			return Color::Black;
		else if (c == "Green")
			return Color::Green;
		else if (c == "Blue")
			return Color::Blue;
		else
			return nullptr;
	}



}