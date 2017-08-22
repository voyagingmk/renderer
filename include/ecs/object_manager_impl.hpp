#ifdef RENDERER_ECS_OBJECT_MANAGER_HPP
#ifndef RENDERER_ECS_OBJECT_MANAGER_CPP
#define RENDERER_ECS_OBJECT_MANAGER_CPP

namespace ecs {

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

#endif
#endif