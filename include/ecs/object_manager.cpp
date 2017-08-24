#include "stdafx.h"
#include "object_manager.hpp"
#include "object.hpp"
#include "event_builtin.hpp"

namespace ecs
{



int ComponentTypeBase::counter = 0;

ObjectManager::ObjectManager(EventManager &event_manager) : m_event_manager(event_manager) {}

ObjectManager::~ObjectManager()
{
	reset();
}

Object ObjectManager::create()
{
	ObjectID id;
	if (m_free_list.size() <= 100000)
	{
		id = ++m_objectIDCounter;
	}
	else
	{
		id = m_free_list.back();
		m_free_list.pop_back();
	}
	isAlive.resize(id + 1);
	isAlive[id] = true;
	Object obj(this, id);
	m_event_manager.emit<ObjectCreatedEvent>(&obj);
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
	m_event_manager.emit<ObjectDestroyedEvent>(id);
	m_free_list.push_back(id);
	isAlive[id] = false;
}

Object ObjectManager::get(ObjectID id)
{
	return Object(this, id);
}
}