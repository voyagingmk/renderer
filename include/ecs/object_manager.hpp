#ifndef RENDERER_ECS_OBJECT_MANAGER_HPP
#define RENDERER_ECS_OBJECT_MANAGER_HPP

#include "setting.hpp"
namespace ecs
{
class Object;

template <typename C>
class ComponentHandle;

class EventManager;

class ComponentTypeBase
{
protected:
	static int counter;
};

template <typename C>
class ComponentType: public ComponentTypeBase
{
public:
	static int typeID()
	{
		static int id = ++counter;
		return id;
	}
};

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
		return isAlive[id];
	}

	Object create();


	Object instantiate(Object original);


	void destroy(ObjectID id);

	Object get(ObjectID id);


	template <typename C, typename... Args>
	ComponentHandle<C> addComponent(ObjectID id, Args &&... args);

	template <typename C>
	MemoryPool<C> *getComponentPool();

	/**
			* Remove a Component from an ObjectID
			*
			* Emits a ComponentRemovedEvent<C> event.
			*/
	template <typename C>
	void remove(ObjectID id);

	/**
			* Check if an Object has a component.
			*/
	template <typename C>
	bool has_component(ObjectID id) const
	{

		ComponentHash h = comHash[id];
		auto typeID = ComponentType<C>::typeID();
		return h.find(typeID) != h.end();
	}

	/**
			* Retrieve a Component assigned to an ObjectID.
			*
			* @returns Pointer to an instance of C, or nullptr if the ObjectID does not have that Component.
			*/
	template <typename C, typename = typename std::enable_if<!std::is_const<C>::value>::type>
	ComponentHandle<C> component(ObjectID id);


	template <typename C, typename = typename std::enable_if<std::is_const<C>::value>::type>
	const ComponentHandle<C> component(ObjectID id) const;



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
	size_t getComponentIdx(ObjectID id);

	template <typename C>
	C *get_component_ptr(ObjectID id);

	template <typename C>
	const C *get_component_ptr(ObjectID id) const;

	uint32_t m_objectIDCounter = 0;

	EventManager &m_event_manager;
	std::vector<uint32_t> m_free_list;
	std::vector<bool> isAlive;

	typedef std::map<size_t, size_t> ComponentHash;
	std::vector<ComponentHash> comHash;
};

template <typename C, typename... Args>
ComponentHandle<C> ObjectManager::addComponent(ObjectID id, Args &&... args)
{

	// Placement new into the component pool.
	MemoryPool<C> *pool = getComponentPool<C>();
	MemoryPool<C>::ElementIdx idx = pool->dispatchIdx();

	pool->newElementByIdx(idx, std::forward<Args>(args)...);

	comHash.resize(id + 1);
	ComponentHash h = comHash[id];
	h[ComponentType<C>::typeID()] = idx;
	comHash[id] = h;
	// Create and return handle.
	ComponentHandle<C> component(this, id);
	m_event_manager.emit<ComponentAddedEvent<C>>(Object(this, id), component);
	return component;
}

template <typename C>
MemoryPool<C>* ObjectManager::getComponentPool()
{
	static MemoryPool<C> pool;
	ComponentType<C>::typeID();
	return &pool;
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

template <typename C>
size_t ObjectManager::getComponentIdx(ObjectID id) {
	auto typeID = ComponentType<C>::typeID();
	ComponentHash h = comHash[id];
	return h[typeID];
}

template <typename C, typename = typename std::enable_if<!std::is_const<C>::value>::type>
ComponentHandle<C> ObjectManager::component(ObjectID id)
{
	auto typeID = ComponentType<C>::typeID();
	ComponentHash h = comHash[id];
	if (h.find(typeID) == h.end()) {
		return ComponentHandle<C>();
	}
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
C *ObjectManager::get_component_ptr(ObjectID id)
{
	assert(valid(id));
	MemoryPool<C> *pool = getComponentPool<C>();
	assert(pool);
	auto idx = getComponentIdx<C>(id);
	return static_cast<C *>(pool->get(idx));
}

template <typename C>
const C *ObjectManager::get_component_ptr(ObjectID id) const
{
	assert(valid(id));
	MemoryPool<C> *pool = getComponentPool<C>();
	assert(pool);
	auto idx = getComponentIdx<C>(id);
	return static_cast<const C *>(pool->get(idx));
}

};

#endif