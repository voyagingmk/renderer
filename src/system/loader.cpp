#include "stdafx.h"
#include "system/loader.hpp"
#include "com/glcommon.hpp"
#include "event/textureEvent.hpp"

using namespace std;

namespace renderer {
	void LoaderSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("LoaderSystem init\n");
        json config = readJson("demo.json");
		string assetsDir = config["assetsDir"];
		string texSubDir = config["texSubDir"];
		string texDir = assetsDir + texSubDir;

		Object obj = objMgr.create(); // manager kinds of resources
		obj.addComponent<TextureDict>();

		loadTextures(evtMgr, obj, texDir, config);
	}

	void LoaderSystem::loadTextures(EventManager &evtMgr, Object obj, string& texDir, json &config)
	{
		for (auto texnfo : config["texture"])
		{
			string fileName = texnfo["file"];
			string aliasName = texnfo["alias"];
			bool hasAlpha = texnfo["hasAlpha"];
			bool toLinear = texnfo["toLinear"];
			evtMgr.emit<LoadTextureEvent>(obj, texDir, fileName.c_str(), aliasName, hasAlpha, toLinear);
		}
	}
};
