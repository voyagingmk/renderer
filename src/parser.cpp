#include "stdafx.h"
#include "parser.hpp"
#include "com/geometry.hpp"
#include "raytracer.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "com/color.hpp"
#include "plane.hpp"
#include "union.hpp"
#include "film.hpp"
#include "com/mesh.hpp"
#include "light.hpp"
#include "com/transform.hpp"
#include "bvh.hpp"
#include "realtime/shadermgr.hpp"
#include "realtime/texturemgr.hpp"

namespace renderer
{

SceneDesc SceneParser::parse(nlohmann::json &config)
{
	return parseSceneDesc(config);
}
SceneDesc SceneParser::parseSceneDesc(nlohmann::json &config)
{
	parseMaterials(config);
	SceneDesc desc(parsePerspectiveCamera(config), parseShapes(config));
	desc.width = config["width"];
	desc.height = config["height"];
	desc.threadsPow = config["threadsPow"];
	desc.maxReflect = config["maxReflect"];
	parseLights(config, desc.lights);
	return desc;
}

void SceneParser::parseShaders(nlohmann::json &config)
{
	ShaderMgrOpenGL &shaderMgr = ShaderMgrOpenGL::getInstance();
	for (auto shaderInfo : config["shader"])
	{
		std::string aliasName = shaderInfo["alias"];
		ShaderProgramHDL spHDL = shaderMgr.createShaderProgram({{ShaderType::Geometry, shaderInfo["gs"].is_null() ? "" : shaderInfo["gs"].get<std::string>()},
																{ShaderType::Vertex, shaderInfo["vs"].get<std::string>()},
																{ShaderType::Fragment, shaderInfo["fs"].get<std::string>()}});
		assert(spHDL);
		if (!spHDL)
		{
			continue;
		}
		shaderMgr.setAlias(spHDL, aliasName.c_str());
	}
}

void SceneParser::parseTextures(nlohmann::json &config)
{
	TextureMgr &texMgr = TextureMgr::getInstance();
	for (auto texnfo : config["texture"])
	{
		std::string fileName = texnfo["file"];
		std::string aliasName = texnfo["alias"];
		bool hasAlpha = texnfo["hasAlpha"];
		bool toLinear = texnfo["toLinear"];

		texMgr.loadTexture(fileName.c_str(), aliasName, hasAlpha, toLinear);
	}
}

void SceneParser::parseMaterials(nlohmann::json &config)
{
	MaterialMgr &mgr = MaterialMgr::getInstance();
	for (auto objinfo : config["material"])
	{
		Material *mt = nullptr;
		MaterialSetting *s = nullptr;
		if (objinfo["type"] == "Phone")
		{
			auto pool1 = GetPool<MaterialPhong>();
			auto pool2 = GetPool<MaterialSettingPhong>();

			s = pool2->newElement(
				parseColor(objinfo["ambient"]),
				parseColor(objinfo["diffuse"]),
				parseColor(objinfo["specular"]),
				objinfo["shininess"],
				objinfo["reflectiveness"]);
			MaterialSettingID sID = mgr.addMaterialSetting(s);

			mt = pool1->newElement();
			mt->bindSetting(sID);
			MaterialID matID = mgr.addMaterial(mt);
			mgr.setMaterialAlias(matID, std::to_string((int)objinfo["id"]));
		}
		else if (objinfo["type"] == "Checker")
		{
			auto pool1 = GetPool<MaterialChecker>();
			auto pool2 = GetPool<MaterialSettingChecker>();

			auto c1 = objinfo["color1"], c2 = objinfo["color2"];
			Color color1 = parseColor(c1, Color::Black), color2 = parseColor(c2, Color::White);
			s = pool2->newElement(color1, color2, objinfo["scale"], objinfo["reflectiveness"]);
			MaterialID sID = mgr.addMaterialSetting(s);

			mt = pool1->newElement();
			mt->bindSetting(sID);
			MaterialID matID = mgr.addMaterial(mt);
			mgr.setMaterialAlias(matID, std::to_string((int)objinfo["id"]));
		}
	}
}

Transform *SceneParser::parseTransform(nlohmann::json &lst)
{
	auto pool_Transform = GetPool<Transform>();
	Transform *o2w = pool_Transform->newElement(Matrix4x4::newIdentity());
	for (auto trans : lst)
	{
		std::string type = trans[0];
		auto data = trans[1];
		Vector3dF v(data[0], data[1], data[2]);
		Transform t;
		if (type == "t")
		{
			//translate
			t = Transform::Translate(v);
		}
		else if (type == "r")
		{
			//rotate
			float angle = data[3];
			v = v.Normalize();
			t = Transform::Rotate(angle, v);
		}
		//TODO: scale
		*o2w = (*o2w) * t;
	}
	return o2w;
}

ShapeUnion *SceneParser::parseShapes(nlohmann::json &config)
{
	auto pool_Transform = GetPool<Transform>();
	Shapes shapes;
	auto addShape = [&](Shape *pShape, int objID, Transform *o2w) {
		if (pShape)
		{
			pShape->id = objID;
			pShape->o2w = o2w;
			pShape->w2o = pool_Transform->newElement(o2w->mInv, o2w->m);
			pShape->Init();
			shapes.push_back(pShape);
		}
	};
	for (auto objinfo : config["obj"])
	{
		int objID = objinfo["Id"];
		Transform *o2w = parseTransform(objinfo["transform"]);
		Shape *pShape = nullptr;
        if (objinfo["type"] == "Sphere")
		{
			float radius = objinfo["radius"];
			auto pool = GetPool<Sphere>();
			pShape = static_cast<Shape *>(pool->newElement(radius));
		}
		else if (objinfo["type"] == "Plane")
		{
			auto pool = GetPool<Plane>();
			float distance = objinfo["distance"];
			pShape = static_cast<Shape *>(pool->newElement(distance));
		}
		else if (objinfo["type"] == "Mesh")
		{
			auto pool = GetPool<Mesh>();
			Vertices vertices;
			UIntArray indexes;
			for (auto i = 0; i < objinfo["vertices"].size(); i++)
			{
				auto position = objinfo["vertices"][i];
				auto normal = objinfo["vnormals"][i];
				auto uv = objinfo["uvs"][i];
				vertices.push_back({Vector3dF(position[0], position[1], position[2]),
									Normal3dF(normal[0], normal[1], normal[2]),
									Vector3dF(0.0f, 0.0f, 0.0f),
									Vector2dF(uv[0], uv[1])});
			}
			for (auto index : objinfo["indexes"])
			{
				indexes.push_back(index);
			}
			Mesh *pMesh = pool->newElement(vertices, indexes);
			if (!objinfo["face"].is_null())
			{
				auto face = objinfo["face"];
				if (face == "front")
				{
					pMesh->face = 0;
				}
				else if (face == "back")
				{
					pMesh->face = 1;
				}
				else if (face == "both")
				{
					pMesh->face = 2;
				}
			}
			if (objinfo["reverse"].is_boolean())
			{
				pMesh->reverse = objinfo["reverse"];
			}
			pShape = static_cast<Shape *>(pMesh);
		}
		else
			continue;

		if (pShape)
		{
			pShape->matAlias = std::to_string((int)objinfo["matId"]);
			addShape(pShape, objID, o2w);
		}
	}
	auto unionPool = GetPool<BVHTree>();
	// auto unionPool = GetPool<ShapeUnion>();
	auto unionObj = unionPool->newElement(shapes);
	return unionObj;
}

void SceneParser::parseLights(nlohmann::json &config, Lights &lights)
{
	for (auto objinfo : config["light"])
	{
		Light *pLight = nullptr;
		bool shadow = objinfo["shadow"].is_null() ? false : (bool)objinfo["shadow"];
		bool softshadow = objinfo["softshadow"].is_null() ? false : (bool)objinfo["softshadow"];
		float radius = objinfo["radius"].is_null() ? 1.0f : (float)objinfo["radius"];
		if (objinfo["type"] == "DirectionLight")
		{
			auto dir = objinfo["dir"];
			auto pool = GetPool<DirectionLight>();
			pLight = static_cast<Light *>(pool->newElement(
				shadow,
				softshadow,
				Vector3dF(dir[0], dir[1], dir[2])));
		}
		else if (objinfo["type"] == "PointLight")
		{
			auto pos = objinfo["pos"];
			int shadowrays = objinfo["shadowrays"].is_null() ? 0 : (int)objinfo["shadowrays"];
			auto pool = GetPool<PointLight>();
			pLight = static_cast<Light *>(pool->newElement(
				Vector3dF(pos[0], pos[1], pos[2]),
				radius,
				shadow,
				softshadow,
				shadowrays));
		}
		if (pLight)
		{
			pLight->ambient = parseColor(objinfo["ambient"]);
			pLight->diffuse = parseColor(objinfo["diffuse"]);
			pLight->specular = parseColor(objinfo["specular"]);
			pLight->constant = objinfo["constant"].is_null() ? 1.0f : (float)objinfo["constant"];
			pLight->linear = objinfo["linear"].is_null() ? 0.014f : (float)objinfo["linear"];
			pLight->quadratic = objinfo["quadratic"].is_null() ? 0.0007f : (float)objinfo["linear"];
			lights.push_back(pLight);
		}
	}
}

PerspectiveCamera SceneParser::parsePerspectiveCamera(nlohmann::json &config)
{
	auto eye = config["camera"]["eye"];
	auto front = config["camera"]["front"];
	auto up = config["camera"]["up"];

	return PerspectiveCamera(Vector3dF(eye[0], eye[1], eye[2]),
							 Vector3dF(Vector3dF(front[0], front[1], front[2]).Normalize()),
							 Vector3dF(Vector3dF(up[0], up[1], up[2]).Normalize()),
							 config["camera"]["fov"]);
}

Color SceneParser::parseColor(nlohmann::json &c)
{
	if (c.is_null())
	{
		return Color::White;
	}
	if (c.is_string())
	{
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
	else
	{
		return Color((float)c[0] / 255.0f, (float)c[1] / 255.0f, (float)c[2] / 255.0f);
	}
}
}
