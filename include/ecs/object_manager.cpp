#include "stdafx.h"
#include "object_manager.hpp"

namespace ecs {

	ObjectManager::ObjectManager(EventManager &event_manager) :
		m_event_manager(event_manager) {}

	ObjectManager::~ObjectManager() {
		reset();
	}

	Object ObjectManager::create()
	{
		ObjectID id;
		if (m_free_list.empty())
		{
			id = m_objectIDCounter++;
		}
		else
		{
			id = m_free_list.back();
			m_free_list.pop_back();
		}
		Object obj(this, id);
		m_event_manager.emit<ObjectCreatedEvent>(&obj);
		return obj;
	}

	Object ObjectManager::create_from_copy(Object original)
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
	}

	Object ObjectManager::get(ObjectID id)
	{
		return Object(this, id);
	}

};