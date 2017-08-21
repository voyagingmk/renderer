#ifndef RENDERER_ECS_OBJECT_MANAGER_HPP
#define RENDERER_ECS_OBJECT_MANAGER_HPP

#include "setting.hpp"
#include "object.hpp"

namespace ecs
{

class EventManager;

class ObjectManager
{
  public:
	explicit ObjectManager(EventManager &event_manager);
	virtual ~ObjectManager();

	/**
		* Number of managed entities.
		*/
	size_t size() const { return 0; }

	/**
		* Current entity capacity.
		*/
	size_t capacity() const { return 0; }

	bool valid(ObjectID id) const
	{
		return id > 0;
	}

	Object create();

	/**
		* Create a new Object by copying another. Copy-constructs each component.
		*
		* Emits ObjectCreatedEvent.
		*/
	Object create_from_copy(Object original);

	/**
		* Destroy an existing ObjectID and its associated Components.
		*
		* Emits ObjectDestroyedEvent.
		*/
	void destroy(ObjectID id);

	Object get(ObjectID id);

	/**
		* Assign a Component to an ObjectID, passing through Component constructor arguments.
		*
		*     Position &position = em.assign<Position>(e, x, y);
		*
		* @returns Smart pointer to newly created component.
		*/
	template <typename C, typename... Args>
	ComponentHandle<C> assign(ObjectID id, Args &&... args)
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
	MemoryPool<C> *getComponentPool()
	{
		static MemoryPool<C> pool;
		return &pool;
	}

	/**
		* Remove a Component from an ObjectID
		*
		* Emits a ComponentRemovedEvent<C> event.
		*/
	template <typename C>
	void remove(ObjectID id)
	{
		MemoryPool<C> *pool = getComponentPool<C>();
		ComponentHandle<C> component(this, id);
		m_event_manager.emit<ComponentRemovedEvent<C>>(Object(this, id), component);

		// Call destructor.
		pool->destroy(id);
	}

	/**
		* Check if an Object has a component.
		*/
	template <typename C>
	bool has_component(ObjectID id) const
	{

		// We don't bother checking the component mask, as we return a nullptr anyway.
		if (family >= component_pools_.size())
			return false;
		if (!pool)
			return false;
		return true;
	}

	/**
		* Retrieve a Component assigned to an ObjectID.
		*
		* @returns Pointer to an instance of C, or nullptr if the ObjectID does not have that Component.
		*/
	template <typename C, typename = typename std::enable_if<!std::is_const<C>::value>::type>
	ComponentHandle<C> component(ObjectID id)
	{
		return ComponentHandle<C>(this, id);
	}

	/**
		* Retrieve a Component assigned to an ObjectID.
		*
		* @returns Component instance, or nullptr if the ObjectID does not have that Component.
		*/
	template <typename C, typename = typename std::enable_if<std::is_const<C>::value>::type>
	const ComponentHandle<C> component(ObjectID id) const
	{

		// We don't bother checking the component mask, as we return a nullptr anyway.
		if (family >= component_pools_.size())
			return ComponentHandle<C, const ObjectManager>();
		MemoryPool<C> *pool = getComponentPool<C>();
		if (!pool)
			return ComponentHandle<C, const ObjectManager>();
		return ComponentHandle<C, const ObjectManager>(this, id);
	}

	/**
		* Destroy all entities and reset the ObjectManager.
		*/
	void reset()
	{
		/*
			for (BasePool *pool : component_pools_)
			{
			if (pool)
			{
			delete pool;
			}
			}
			component_pools_.clear();
			*/
		m_free_list.clear();
		m_objectIDCounter = 0;
	}

  private:
	friend class Object;
	template <typename C>
	friend class ComponentHandle;

	template <typename C>
	C *get_component_ptr(ObjectID id)
	{
		assert(valid(id));
		MemoryPool<C> *pool = getComponentPool<C>();
		assert(pool);
		return static_cast<C *>(pool->get(id));
	}

	template <typename C>
	const C *get_component_ptr(ObjectID id) const
	{

		MemoryPool<C> *pool = getComponentPool<C>();
		assert(pool);
		return static_cast<const C *>(pool->get(id));
	}

	uint32_t m_objectIDCounter = 0;

	EventManager &m_event_manager;
	std::vector<uint32_t> m_free_list;
};
};

#endif