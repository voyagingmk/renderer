#include "stdafx.h"
#include "object_manager.hpp"
#include "object.hpp"
#include "event_builtin.hpp"

namespace ecs
{

int ComponentTypeBase::counter = 0;

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
	m_evtMgr.emit<ObjectCreatedEvent>(&obj);
	return obj;
}

Object ObjectManager::instantiate(Object original)
{
	assert(original.valid());
	auto clone = create();
	// TODO
	return clone;
}

void ObjectManager::destroy(ObjectID id)
{
	// TODO com delete
	ComponentHash h = m_comHashes[id];
	for (auto idx : h) {
		idx.second;
	}
	m_evtMgr.emit<ObjectDestroyedEvent>(id);
	m_freeList.push_back(id);
	m_isAlive[id] = false;
}

Object ObjectManager::get(ObjectID id)
{
	return Object(this, id);
}
}