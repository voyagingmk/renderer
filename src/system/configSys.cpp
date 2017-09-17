#include "stdafx.h"
#include "system/configSys.hpp"

using json = nlohmann::json;
using namespace std;

namespace renderer {

	void ConfigSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
		printf("ConfigSystem init\n"); 
		json config = readJson("demo.json");
		evtMgr.emit<LoadConfigEvent>(config);

    }

	void ConfigSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        
    }


	void ConfigSystem::receive(const LoadConfigEvent& evt) {

	}


};