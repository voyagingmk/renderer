#include "stdafx.h"
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
#include "transform.hpp"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

namespace renderer {
	bool parserObj(const std::string& objFilePath, std::vector<tinyobj::shape_t>& shapes, std::vector<tinyobj::material_t>& materials);


	SceneDesc SceneParser::parse(nlohmann::json& config) {
		return parseSceneDesc(config);
	}
	SceneDesc SceneParser::parseSceneDesc(nlohmann::json& config) {
		MaterialDict matDict;
		parseMaterials(config, matDict); 
		SceneDesc desc(parsePerspectiveCamera(config), parseShapes(config, matDict));
		desc.matDict = std::move(matDict);
		desc.width = config["width"];
		desc.height = config["height"];
		desc.threadsPow = config["threadsPow"];
		desc.maxReflect = config["maxReflect"];
		parseLights(config, desc.lights, desc.matDict);
		return desc;
	}

	void SceneParser::parseMaterials(nlohmann::json& config, MaterialDict& matDict)
	{
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
				auto c1 = objinfo["color1"], c2 = objinfo["color2"];
				Color color1 = parseColor(c1, Color::Black), color2 = parseColor(c2, Color::White);
				mt = pool->newElement(objinfo["scale"], objinfo["reflectiveness"], color1, color2);
			}
			matDict[objinfo["id"]] = mt;
		}
	}

	Transform* SceneParser::parseTransform(nlohmann::json& lst)
	{
		auto pool_Transform = GetPool<Transform>();
		Transform* o2w = pool_Transform->newElement(Matrix4x4::newIdentity());
		for (auto trans : lst) {
			std::string type = trans[0];
			auto data = trans[1];
			Vector3dF v(data[0], data[1], data[2]);
			Transform t;
			if (type == "t") {
				//translate
				t = Translate(v);
			}
			else if (type == "r") {
				//rotate
				float angle = data[3];
				v = v.Normalize();
				t = Rotate(angle, v);
			}
			//TODO: scale
			*o2w = (*o2w) * t;
		}
		return o2w;
	}

	ShapeUnion SceneParser::parseShapes(nlohmann::json& config, MaterialDict& matDict)
	{
		auto pool_Transform = GetPool<Transform>();
		Shapes shapes;
		for (auto objinfo : config["obj"]) {
			Transform* o2w = parseTransform(objinfo["transform"]);
			Shape* pShape = nullptr;
			if (objinfo["type"] == "3dsMax") {
				const std::string& filepath = objinfo["filepath"];
				std::vector<tinyobj::shape_t> shapes;
				std::vector<tinyobj::material_t> materials;
				parserObj(filepath, shapes, materials);
				for (size_t i = 0; i < shapes.size(); i++) {
					auto pool = GetPool<Mesh>();
					VectorArray vertices;
					NormalArray normals;
					UIntArray indexes;
					UVArray uvs;
					// printf("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
					// assert((shapes[i].mesh.positions.size() % 3) == 0);
					auto positions = shapes[i].mesh.positions;
					for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
						// x->z y->x z->y
						// 0->2 1->0 2->1
						float z = positions[3 * v + 0];
						float x = positions[3 * v + 1];
						float y = positions[3 * v + 2];
						vertices.push_back(Vector3dF(x, y, z));
						printf("v[%d] = %.1f, %.1f, %.1f\n", v, x, y, z);
					}
					// assert((shapes[i].mesh.indices.size() % 3) == 0);
					// printf("Size of shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
					auto indices = shapes[i].mesh.indices;
					for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
						indexes.push_back(indices[3 * f + 0]);
						indexes.push_back(indices[3 * f + 1]);
						indexes.push_back(indices[3 * f + 2]);
						
						printf("idx[%ld] = %d, %d, %d\n", f, shapes[i].mesh.indices[3 * f + 0] + 1, 
							shapes[i].mesh.indices[3 * f + 1] + 1, shapes[i].mesh.indices[3 * f + 2] + 1);
					}
					normals.resize(vertices.size());
					/*
					int idx = 0;
					for (auto normal : objinfo["normals"]) {
						normals[idx++] = Normal3dF(normal[0], normal[1], normal[2]);
					}
					*/
					Mesh* pMesh = pool->newElement(vertices, normals, indexes, uvs);
					pShape = static_cast<Shape*>(pMesh);
				}
				pShape->material = matDict[objinfo["matId"]];
			}
			else if (objinfo["type"] == "Sphere") {
				float radius  = objinfo["radius"];
				auto pool = GetPool<Sphere>();
				pShape = static_cast<Shape*>(pool->newElement(radius));
				pShape->material = matDict[objinfo["matId"]];
			}
			else if (objinfo["type"] == "Plane") {
				auto pool = GetPool<Plane>();
				float distance = objinfo["distance"];
				pShape = static_cast<Shape*>(pool->newElement(distance));
				pShape->material = matDict[objinfo["matId"]];
			}
			else if (objinfo["type"] == "Mesh") {
				auto pool = GetPool<Mesh>();
				VectorArray vertices;
				NormalArray normals;
				UIntArray indexes;
				UVArray uvs;
				for (auto vertice : objinfo["vertices"]) {
					vertices.push_back(Vector3dF(vertice[0], vertice[1], vertice[2]));
				}
				for (auto index : objinfo["indexes"]) {
					indexes.push_back(index);
				}
				normals.resize(indexes.size());
				int idx = 0;
				for (auto normal : objinfo["normals"]) {
					normals[idx++] = Normal3dF(normal[0], normal[1], normal[2]);
				}
				for (auto uv : objinfo["uvs"]) {
					uvs.push_back(Vector2dF(uv[0], uv[1]));
				}
				Mesh* pMesh = pool->newElement(vertices, normals, indexes, uvs);
				if (!objinfo["face"].is_null()) {
					auto face = objinfo["face"];
					if (face == "front") {
						pMesh->face = 0;
					}
					else if (face == "back") {
						pMesh->face = 1;
					}
					else if(face == "both"){
						pMesh->face = 2;
					}
				}
				if (objinfo["reverse"].is_boolean()) {
					pMesh->reverse = objinfo["reverse"];
				}
				pShape = static_cast<Shape*>(pMesh);
				pShape->material = matDict[objinfo["matId"]];
			}
			else
				continue;
			if (pShape) {
				pShape->o2w = o2w;
				pShape->w2o = pool_Transform->newElement(o2w->mInv, o2w->m);
				pShape->Init();
				shapes.push_back(pShape);
			}

		}
		return ShapeUnion(shapes);
	}

	void SceneParser::parseLights(nlohmann::json& config, Lights& lights, MaterialDict& matDict)
	{
		for (auto objinfo : config["light"]) {
			Light* pLight = nullptr;
			if (objinfo["type"] == "DirectionLight") {
				auto dir = objinfo["dir"];
				auto color = objinfo["color"];
				auto pool = GetPool<DirectionLight>();
				pLight = static_cast<Light*>(pool->newElement(Vector3dF(dir[0], dir[1], dir[2])));
				pLight->lightType = LightType_Direction;
			}
			else if (objinfo["type"] == "PointLight") {
				auto pos = objinfo["pos"];
				auto color = objinfo["color"];
				auto radius = objinfo["radius"];
				auto shadowrays = objinfo["shadowrays"];
				auto shadow = objinfo["shadow"];
				auto softshadow = objinfo["softshadow"];
				auto pool = GetPool<PointLight>();
				pLight = static_cast<Light*>(pool->newElement(
					Vector3dF(pos[0], pos[1], pos[2]),
					shadow,
					softshadow,
					radius,
					shadowrays));
				pLight->lightType = LightType_Point;
			}
			if (pLight)
				lights.push_back(pLight);
		}
	}

	PerspectiveCamera SceneParser::parsePerspectiveCamera(nlohmann::json& config)
	{
		auto eye = config["camera"]["eye"];
		auto front = config["camera"]["front"];
		auto up = config["camera"]["up"];

		return PerspectiveCamera(Vector3dF(eye[0], eye[1], eye[2]),
			Vector3dF(Vector3dF(front[0], front[1], front[2]).Normalize()),
			Vector3dF(Vector3dF(up[0], up[1], up[2]).Normalize()),
			config["camera"]["fov"]);
	}

	Color SceneParser::parseColor(nlohmann::json& c) {
		if (c.is_string()) {
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
		else {
			return Color(c[0] / 255.0f, c[1] / 255.0f, c[2] / 255.0f);
		}
	}

	bool parserObj(const std::string& objFilePath, std::vector<tinyobj::shape_t>& shapes, std::vector<tinyobj::material_t>& materials) {
		std::string err;
		bool ret = tinyobj::LoadObj(shapes, materials, err, objFilePath.c_str(), "./");

		if (!err.empty()) { // `err` may contain warning message.
			std::cerr << err << std::endl;
		}

		if (!ret) {
			return false;
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
		return true;
	}

}