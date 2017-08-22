#include "stdafx.h"
#include "system.hpp"

namespace ecs {
	BaseSystem::~BaseSystem() {

	}

	BaseSystem::TypeID BaseSystem::m_SystemTypeCounter = 0;

	void SystemManager::update_all(float dt) {

	}
	void SystemManager::configure() {

	}
};

