#include "stdafx.h"
#include "system.hpp"

namespace ecs {
	BaseSystem::~BaseSystem() {

	}

	BaseSystem::TypeID BaseSystem::m_SystemTypeCounter = 0;

	void SystemManager::update_all(float dt) {
		assert(initialized_ && "SystemManager::configure() not called");
		for (auto &pair : systems_) {
			pair.second->update(entity_manager_, event_manager_, dt);
		}
	}

	void SystemManager::configure() {
		for (auto &pair : systems_) {
			pair.second->configure(entity_manager_, event_manager_);
		}
		initialized_ = true;
	}
};

