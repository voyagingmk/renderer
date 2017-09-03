#ifndef RENDERER_ECS_OBJECT_MANAGER_HPP
#define RENDERER_ECS_OBJECT_MANAGER_HPP

#include "setting.hpp"
namespace ecs
{
class Object;

template <typename C>
class ComponentHandle;

class EventManager;

typedef size_t ComponentTypeID;

class ComponentTypeBase
{
  protected:
	static ComponentTypeID typeIDCounter;
};

template <typename C>
class ComponentType : public ComponentTypeBase
{
  public:
	static ComponentTypeID typeID()
	{
		static ComponentTypeID typeID = ++typeIDCounter;
		return typeID;
	}
};

class ObjectManager
{
  public:
	explicit ObjectManager(EventManager &evtMgr);
	virtual ~ObjectManager();

	size_t size() const { return 0; }

	size_t capacity() const { return 0; }

	bool valid(ObjectID id) const
	{
		return m_isAlive[id];
	}

	Object create();

	Object instantiate(Object original);

	void destroy(ObjectID id);

	Object get(ObjectID id);

	template <typename C, typename... Args>
	ComponentHandle<C> addComponent(ObjectID id, Args &&... args);

	template <typename C>
	MemoryPool<C> *getComponentPool() const;

	template <typename C>
	void removeComponent(ObjectID id);

	template <typename C>
	bool hasComponent(ObjectID id) const
	{

		return getComponentIdx<C>(id) != -1;
	}

	template <typename C>
	ComponentHandle<C> component(ObjectID id) const;

	void reset()
	{
		for (ObjectID id = 0, len = m_isAlive.size(); id < len; id++)
		{
			if (m_isAlive[id])
			{
				destroy(id);
			}
		}
		m_freeList.clear();
		m_isAlive.clear();
		m_comHashes.clear();
		m_objectIDCounter = 0;
	}

  private:
	friend class Object;
	template <typename C>
	friend class ComponentHandle;

	template <typename C>
	size_t getComponentIdx(ObjectID id) const;

	template <typename C>
	C *getComponentPtr(ObjectID id);

	template <typename C>
	const C *getComponentPtr(const ObjectID id) const;

	uint32_t m_objectIDCounter = 0;

	EventManager &m_evtMgr;
	std::vector<uint32_t> m_freeList;
	std::vector<bool> m_isAlive;

	typedef std::map<ComponentTypeID, size_t> ComponentHash;
	std::vector<ComponentHash> m_comHashes;

	struct ComponentMetaInfo
	{
		MemoryPoolBase *pool = nullptr;
	};

	std::map<ComponentTypeID, ComponentMetaInfo> m_comMetaInfo;
};

template <typename C, typename... Args>
ComponentHandle<C> ObjectManager::addComponent(ObjectID id, Args &&... args)
{

	// Placement new into the component pool.
	MemoryPool<C> *pool = getComponentPool<C>();
	ComponentTypeID typeID = ComponentType<C>::typeID();
	if (m_comMetaInfo.find(typeID) == m_comMetaInfo.end()) {
		ComponentMetaInfo info;
		info.pool = pool;
		m_comMetaInfo[typeID] = info;
	}

	if (m_comHashes.size() <= id) {
		m_comHashes.resize(id + 1);
	}
	ComponentHash h = m_comHashes[id];
	if (h.find(typeID) != h.end()) {
		// already exists
		ComponentHandle<C> component(this, id);
		return component;
	}
	MemoryPool<C>::ElementIdx idx = pool->dispatchIdx();
	pool->newElementByIdx(idx, std::forward<Args>(args)...);
	h[typeID] = idx;
	m_comHashes[id] = h;
	// Create and return handle.
	ComponentHandle<C> component(this, id);
	m_evtMgr.emit<ComponentAddedEvent<C>>(Object(this, id), component);
	return component;
}

template <typename C>
MemoryPool<C> *ObjectManager::getComponentPool() const
{
	static MemoryPool<C> pool;
	ComponentTypeID typeID = ComponentType<C>::typeID();
	return &pool;
}

template <typename C>
void ObjectManager::removeComponent(ObjectID id)
{
	size_t idx = getComponentIdx<C>(id);
	MemoryPool<C> *pool = getComponentPool<C>();
	ComponentHandle<C> component(this, id);
	m_evtMgr.emit<ComponentRemovedEvent<C>>(Object(this, id), component);
	pool->deleteElementByIdx(idx);
	ComponentHash &h = m_comHashes[id];
	auto typeID = ComponentType<C>::typeID();
	auto it = h.find(typeID);
	assert(it != h.end());
	h.erase(it);
}

template <typename C>
size_t ObjectManager::getComponentIdx(ObjectID id) const
{
	auto typeID = ComponentType<C>::typeID();
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

template <typename C>
ComponentHandle<C> ObjectManager::component(ObjectID id) const
{
	auto idx = getComponentIdx<C>(id);
	if (idx == -1)
	{
		return ComponentHandle<C>();
	}
	return ComponentHandle<C>(this, id);
}

template <typename C>
C *ObjectManager::getComponentPtr(ObjectID id)
{
	assert(valid(id));
	MemoryPool<C> *pool = getComponentPool<C>();
	assert(pool);
	auto idx = getComponentIdx<C>(id);
	return static_cast<C *>(pool->get(idx));
}

template <typename C>
const C *ObjectManager::getComponentPtr(const ObjectID id) const
{
	assert(valid(id));
	MemoryPool<C> *pool = getComponentPool<C>();
	assert(pool);
	auto idx = getComponentIdx<C>(id);
	return static_cast<const C *>(pool->get(idx));
}
};

#endif