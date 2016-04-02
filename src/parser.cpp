#include "stdafx.h"
#include <fstream>
#include <map>
#include <stdio.h>
#include <thread>
#include <ctime>
#include <chrono>
#include "parser.hpp"
#include "geometry.hpp"
#include "renderer.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "color.hpp"
#include "plane.hpp"
#include "union.hpp"
#include "film.hpp"
#include "mesh.hpp"
#include "light.hpp"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

namespace renderer {

	void renderArea(Renderer &renderer, Film* film, Shape* pUnion, PerspectiveCamera& camera, Vector3dF& lightDir, int maxReflect, int x, int y, int w, int h)
	{
		renderer.rayTraceReflection(film, pUnion, camera, lightDir, maxReflect, x, y, w, h);
	}

	int Parser::parseFromFile(std::string path, Film * film) {
		using json = nlohmann::json;
		std::ifstream f("config.json");
		std::string config_string((std::istreambuf_iterator<char>(f)),
			(std::istreambuf_iterator<char>()));
		f.close();
		json config = json::parse(config_string);

		int width = config["width"], height = config["height"], multithread = config["multithread"];
		printf("width: %d, height: %d, multithread: %d \n", width, height, multithread);
		if(film->width()!=width || film->height()!=height)
			film->resize(width, height);

		std::map<int, Material*> matDict;

		for (auto objinfo : config["material"]) {
			Material* mt = nullptr;
			if (objinfo["type"] == "Phone") {
				auto pool = GetPool<PhongMaterial>();
				mt = pool->newElement(parseColor(objinfo["diffuse"]),
						parseColor(objinfo["specular"]),
						objinfo["shininess"],
						objinfo["reflectiveness"]);
			}
			else if (objinfo["type"] == "Checker") {
				auto pool = GetPool<CheckerMaterial>();
				mt = pool->newElement(objinfo["scale"], objinfo["reflectiveness"]);
			}
			matDict[objinfo["id"]] = mt;
		}
		int maxReflect = config["maxReflect"];

		std::vector<Light*> lights;
		Vector3dF lightDir(0,0,1);
		std::vector<Shape*> vecGeo;
		for (auto objinfo : config["scene"]) {
			Shape* pg = nullptr;
			Light* light = nullptr;
			if (objinfo["type"] == "DirectionLight") {
				auto dir = objinfo["dir"];
				auto color = objinfo["color"];
				auto pool = GetPool<DirectionLight>();
				light = static_cast<Light*>(pool->newElement(Vector3dF(dir[0], dir[1], dir[2])));
			}
			else if (objinfo["type"] == "PointLight") {
				auto pos = objinfo["pos"];
				auto color = objinfo["color"];
				auto pool = GetPool<PointLight>();
				light = static_cast<Light*>(pool->newElement(Vector3dF(pos[0], pos[1], pos[2])));
			}
			else if (objinfo["type"] == "Sphere") {
				auto pos = objinfo["pos"];
				auto pool = GetPool<Sphere>();
				pg = pool->newElement(Vector3dF(pos[0], pos[1], pos[2]),
					objinfo["radius"]);
				pg->material = matDict[objinfo["matId"]];
			}
			else if (objinfo["type"] == "Plane") {
				auto normal = objinfo["normal"];
				auto pool = GetPool<Plane>();
				pg = pool->newElement(Vector3dF(normal[0], normal[1], normal[2]),
					   objinfo["distance"] );
				pg->material = matDict[objinfo["matId"]];
			}
			else if (objinfo["type"] == "Mesh") {
				auto pool = GetPool<Mesh>();
				VectorArray vertices, normals;
				UIntArray indexes;
				UVArray uvs;
				for (auto vertice : objinfo["vertices"]) {
					vertices.push_back(Vector3dF(vertice[0], vertice[1], vertice[2]));
				}
				for (auto normal : objinfo["normals"]) {
					normals.push_back(Vector3dF(normal[0], normal[1], normal[2]));
				}
				for (auto index : objinfo["indexes"]) {
					indexes.push_back(index);
				}
				for (auto uv : objinfo["uvs"]) {
					uvs.push_back(Vector2dF(uv[0], uv[1]));
				}
				pg = pool->newElement(vertices, normals, indexes, uvs);
				pg->material = matDict[objinfo["matId"]];
			}
			else
				continue;
			if (light)
				lights.push_back(light);
			if(pg)
				vecGeo.push_back(pg);
		} 
		ShapeUnion shapeUnion(vecGeo);

		auto eye = config["camera"]["eye"];
		auto front = config["camera"]["front"];
		auto up = config["camera"]["up"];

		PerspectiveCamera camera(Vector3dF(eye[0], eye[1], eye[2]),
			Vector3dF(front[0], front[1], front[2]).Normalize(),
			Vector3dF(up[0], up[1], up[2]).Normalize(),
			config["camera"]["fov"]);

		Renderer renderer;

		auto time0 = std::chrono::system_clock::now();
		if (!multithread) {
			renderer.rayTrace(film, shapeUnion, camera, lightDir);
			renderer.rayTraceReflection(film, &shapeUnion, camera, std::ref(lightDir), 4);
		}
		else {
			int threads_num = int(pow(2.0, multithread));
			std::thread* *threads = new std::thread*[threads_num];
			for (int i = 0; i < threads_num; i++) {
				std::thread* t = new std::thread(renderArea, std::ref(renderer), std::ref(film), &shapeUnion, std::ref(camera), std::ref(lightDir), maxReflect, 0, i * height / threads_num, width, height / threads_num);
				threads[i] = t;
			}
			for (int i = 0; i < threads_num; i++) {
				threads[i]->join();
			}
		}

		//parserObj(config["obj"]);
		auto time1 = std::chrono::system_clock::now();
		auto time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
		printf("cost: %lldms\n", time_cost);
		//img.display("");
		return 0;
	}

	Color Parser::parseColor(std::string c) {
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
			return Color::White;
	}

	void Parser::parserObj(std::string inputfile) {
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;
		bool ret = tinyobj::LoadObj(shapes, materials, err, inputfile.c_str());

		if (!err.empty()) { // `err` may contain warning message.
			std::cerr << err << std::endl;
		}

		if (!ret) {
			exit(1);
		}

		std::cout << "# of shapes    : " << shapes.size() << std::endl;
		std::cout << "# of materials : " << materials.size() << std::endl;

		for (size_t i = 0; i < shapes.size(); i++) {
			printf("shape[%ld].name = %s\n", i, shapes[i].name.c_str());
			printf("Size of shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
			printf("Size of shape[%ld].material_ids: %ld\n", i, shapes[i].mesh.material_ids.size());
			assert((shapes[i].mesh.indices.size() % 3) == 0);
			for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
				printf("  idx[%ld] = %d, %d, %d. mat_id = %d\n", f, shapes[i].mesh.indices[3 * f + 0], shapes[i].mesh.indices[3 * f + 1], shapes[i].mesh.indices[3 * f + 2], shapes[i].mesh.material_ids[f]);
			}

			printf("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
			assert((shapes[i].mesh.positions.size() % 3) == 0);
			for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
				printf("  v[%ld] = (%f, %f, %f)\n", v,
					shapes[i].mesh.positions[3 * v + 0],
					shapes[i].mesh.positions[3 * v + 1],
					shapes[i].mesh.positions[3 * v + 2]);
			}
		}

		for (size_t i = 0; i < materials.size(); i++) {
			printf("material[%ld].name = %s\n", i, materials[i].name.c_str());
			printf("  material.Ka = (%f, %f ,%f)\n", materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
			printf("  material.Kd = (%f, %f ,%f)\n", materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
			printf("  material.Ks = (%f, %f ,%f)\n", materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
			printf("  material.Tr = (%f, %f ,%f)\n", materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);
			printf("  material.Ke = (%f, %f ,%f)\n", materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
			printf("  material.Ns = %f\n", materials[i].shininess);
			printf("  material.Ni = %f\n", materials[i].ior);
			printf("  material.dissolve = %f\n", materials[i].dissolve);
			printf("  material.illum = %d\n", materials[i].illum);
			printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
			printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
			printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
			printf("  material.map_Ns = %s\n", materials[i].specular_highlight_texname.c_str());
			std::map<std::string, std::string>::const_iterator it(materials[i].unknown_parameter.begin());
			std::map<std::string, std::string>::const_iterator itEnd(materials[i].unknown_parameter.end());
			for (; it != itEnd; it++) {
				printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
			}
			printf("\n");
		}
	}

}