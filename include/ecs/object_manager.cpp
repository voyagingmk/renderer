#include "stdafx.h"
#include "object_manager.hpp"
#include "component_handle.hpp"
#include "event.hpp"
#include "event_builtin.hpp"


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

	template <typename C, typename... Args>
	ComponentHandle<C> ObjectManager::assign(ObjectID id, Args &&... args)
	{

		// Placement new into the component pool.
		MemoryPool<C> *pool = getComponentPool<C>();
		pool->newElementByIdx(id, std::forward<Args>(args)...);

		// Create and return handle.
		ComponentHandle<C> component(this, id);
		m_event_manager.emit<ComponentAddedEvent<C>>(Object(this, id), component);
		return component;
	}

	template <typename C>
	void ObjectManager::remove(ObjectID id)
	{
		MemoryPool<C> *pool = getComponentPool<C>();
		ComponentHandle<C> component(this, id);
		m_event_manager.emit<ComponentRemovedEvent<C>>(Object(this, id), component);

		// Call destructor.
		pool->destroy(id);
	}

	template <typename C, typename = typename std::enable_if<!std::is_const<C>::value>::type>
	ComponentHandle<C> ObjectManager::component(ObjectID id) {
		return ComponentHandle<C>(this, id);
	}

	template <typename C, typename = typename std::enable_if<std::is_const<C>::value>::type>
	const ComponentHandle<C> ObjectManager::component(ObjectID id) const
	{

		// We don't bother checking the component mask, as we return a nullptr anyway.
		if (family >= component_pools_.size())
			return ComponentHandle<C, const ObjectManager>();
		MemoryPool<C> *pool = getComponentPool<C>();
		if (!pool)
			return ComponentHandle<C, const ObjectManager>();
		return ComponentHandle<C, const ObjectManager>(this, id);
	}

	template <typename C>
	C* ObjectManager::get_component_ptr(ObjectID id)
	{
		assert(valid(id));
		MemoryPool<C> *pool = getComponentPool<C>();
		assert(pool);
		return static_cast<C *>(pool->get(id));
	}

	template <typename C>
	const C* ObjectManager::get_component_ptr(ObjectID id) const
	{

		MemoryPool<C> *pool = getComponentPool<C>();
		assert(pool);
		return static_cast<const C *>(pool->get(id));
	}
};