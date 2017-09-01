#include "stdafx.h"
#include "system.hpp"

namespace ecs
{
BaseSystem::~BaseSystem()
{
}

BaseSystem::TypeID BaseSystem::m_SystemTypeCounter = 0;

void SystemManager::update_all(float dt)
{
	assert(m_inited && "SystemManager::configure() not called");
	for (auto &pair : m_evtTypeID2System)
	{
		pair.second->update(objMgr, evtMgr, dt);
	}
}

void SystemManager::configure()
{
	for (auto &pair : m_evtTypeID2System)
	{
		pair.second->configure(objMgr, evtMgr);
	}
	m_inited = true;
}
};
