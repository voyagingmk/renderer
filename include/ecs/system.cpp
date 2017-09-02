#include "stdafx.h"
#include "system.hpp"

namespace ecs
{
BaseSystem::~BaseSystem()
{
}

BaseSystem::TypeID BaseSystem::m_SystemTypeCounter = 0;

void SystemManager::updateAll(float dt)
{
	assert(m_inited && "SystemManager::init() not called");
	for (auto &pair : m_evtTypeID2System)
	{
		pair.second->update(m_objMgr, m_evtMgr, dt);
	}
}

void SystemManager::init()
{
	for (auto &pair : m_evtTypeID2System)
	{
		pair.second->init(m_objMgr, m_evtMgr);
	}
	m_inited = true;
}
};
