#include "stdafx.h"
#include "object_manager.hpp"
#include "object.hpp"
#include "event_builtin.hpp"

namespace ecs
{

ComponentTypeID ComponentTypeBase::typeIDCounter = 0;

ObjectManager::ObjectManager(EventManager &evtMgr) : m_evtMgr(evtMgr) {}

ObjectManager::~ObjectManager()
{
	reset();
}

Object ObjectManager::create()
{
	ObjectID id;
	if (m_freeList.size() <= 100000)
	{
		id = ++m_objectIDCounter;
	}
	else
	{
		id = m_freeList.back();
		m_freeList.pop_back();
	}
	m_isAlive.resize(id + 1);
	m_isAlive[id] = true;
	Object obj(this, id);
	m_evtMgr.emit<ObjectCreatedEvent>(obj);
	return obj;
}


void ObjectManager::destroy(ObjectID id)
{
	// TODO com delete
	ComponentHash h = m_comHashes[id];
	for (auto pair : h) {
		ComponentTypeID typeID = pair.first;
		size_t idx = pair.second;
		ComponentMetaInfo& info = m_comMetaInfo[typeID];
		info.pool->deleteElementByIdx(idx);
		ObjectIDs& objectIDs = m_objectIDs[typeID];
		objectIDs.erase(id);
	}
	m_evtMgr.emit<ObjectDestroyedEvent>(id);
	m_freeList.push_back(id);
	m_isAlive[id] = false;
}

Object ObjectManager::get(ObjectID id)
{
	return Object(this, id);
}

size_t ObjectManager::getComponentIdx(ComponentTypeID typeID, ObjectID id) const
{
	if (id >= m_comHashes.size())
	{
		return -1;
	}
	const ComponentHash &h = m_comHashes[id];
	auto it2 = h.find(typeID);
	if (it2 == h.end())
	{
		return -1;
	}
	return it2->second;
}
}

