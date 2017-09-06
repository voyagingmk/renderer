#ifndef RENDERER_ECS_OBJECT_MANAGER_HPP
#define RENDERER_ECS_OBJECT_MANAGER_HPP

#include "setting.hpp"
#include "../MemoryPool.hpp"
#include "event.hpp"
#include "event_builtin.hpp"

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

	size_t size() const { return m_isAlive.size() - m_freeList.size(); }

	size_t capacity() const { return m_isAlive.size(); }

	bool valid(ObjectID id) const
	{
		return m_isAlive[id];
	}

	Object create();

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

  private:
	uint32_t m_objectIDCounter = 0;

	EventManager &m_evtMgr;
	std::vector<uint32_t> m_freeList;
	std::vector<bool> m_isAlive;

	typedef std::map<ComponentTypeID, size_t> ComponentHash;
	std::vector<ComponentHash> m_comHashes;

	typedef std::set<ObjectID> ObjectIDs;
	std::vector<ObjectIDs> m_ObjectIDs;

	struct ComponentMetaInfo
	{
		MemoryPoolBase *pool = nullptr;
	};

	std::map<ComponentTypeID, ComponentMetaInfo> m_comMetaInfo;

  public:
	class BaseView {
	public:
		class ViewIterator : public std::iterator<std::input_iterator_tag, Object> {
			public:
				ViewIterator &operator ++() {
					++m_idx;
					next();
					return *this;
				}
				bool operator == (const ViewIterator& rhs) const { return m_idx == rhs.m_idx; }
				bool operator != (const ViewIterator& rhs) const { return m_idx != rhs.m_idx; }
				Object operator * () { return Object(m_ObjMgr, *m_idx); }
				const Object operator * () const { return Object(m_ObjMgr, *m_idx); }

			public:
				ViewIterator(ObjectManager *objMgr, ObjectIDs::iterator idx)
					: m_ObjMgr(objMgr), m_idx(idx), m_capacity(m_ObjMgr->capacity()) {
				}

				void next() {
				}

				inline bool predicate() {
					return true;
				}

				inline bool valid_entity() {
					return true;
				}

				ObjectManager *m_ObjMgr;
				ObjectIDs::iterator m_idx;
				size_t m_capacity;
		};

		ViewIterator begin() { return ViewIterator(m_ObjMgr, m_ObjMgr->m_ObjectIDs[typeID].begin()); }
		ViewIterator end() { return ViewIterator(m_ObjMgr, m_ObjMgr->m_ObjectIDs[typeID].end()); }
		const ViewIterator begin() const { return ViewIterator(m_ObjMgr, m_ObjMgr->m_ObjectIDs[typeID].begin()); }
		const ViewIterator end() const { return ViewIterator(m_ObjMgr, m_ObjMgr->m_ObjectIDs[typeID].end()); }

	private:
		friend class ObjectManager;

		BaseView(ObjectManager *manager) :
			m_ObjMgr(manager) {}

		ObjectManager *m_ObjMgr;
		ComponentTypeID typeID;
	};
};

    
    template <typename C>
    void Object::removeComponent()
    {
        assert(valid() && hasComponent<C>());
        m_manager->removeComponent<C>(m_id);
    }
    
    template <typename C>
    ComponentHandle<C> Object::component() const
    {
        assert(valid());
        return m_manager->component<C>(m_id);
    }
    
    template <typename C, typename... Args>
    ComponentHandle<C> Object::addComponent(Args &&... args)
    {
        assert(valid());
        return m_manager->addComponent<C>(m_id, std::forward<Args>(args)...);
    }
    
    template <typename C, typename... Args>
    ComponentHandle<C> Object::replace(Args &&... args)
    {
        assert(valid());
        auto handle = component<C>();
        if (handle)
        {
            *(handle.get()) = C(std::forward<Args>(args)...);
        }
        else
        {
            handle = m_manager->addComponent<C>(m_id, std::forward<Args>(args)...);
        }
        return handle;
    }
    
    template <typename C>
    bool Object::hasComponent() const
    {
        assert(valid());
        return m_manager->hasComponent<C>(m_id);
    }
    
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
	auto idx = pool->dispatchIdx();
	pool->newElementByIdx(idx, std::forward<Args>(args)...);
	h[typeID] = idx;
	m_comHashes[id] = h;
	if (m_ObjectIDs.size() <= typeID) {
		m_ObjectIDs.resize(typeID + 1);
	}
	ObjectIDs& objectIDs = m_ObjectIDs[typeID];
	objectIDs.insert(id);

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
	if (idx == -1) {
		return;
	}
	MemoryPool<C> *pool = getComponentPool<C>();
	ComponentHandle<C> component(this, id);
	m_evtMgr.emit<ComponentRemovedEvent<C>>(Object(this, id), component);
	pool->deleteElementByIdx(idx);
	ComponentHash &h = m_comHashes[id];
	auto typeID = ComponentType<C>::typeID();
	auto it = h.find(typeID);
	assert(it != h.end());
	ObjectIDs& objectIDs = m_ObjectIDs[typeID];
	objectIDs.erase(id);
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
    
    
    template <typename C>
    bool ComponentHandle<C>::valid() const
    {
        return m_manager && m_manager->valid(m_id) && m_manager->template hasComponent<C>(m_id);
    }
    
    template <typename C>
    ComponentHandle<C>::operator bool() const
    {
        return valid();
    }
    
    template <typename C>
    inline C *ComponentHandle<C>::operator->()
    {
        assert(valid());
        return const_cast<C *>(m_manager->template getComponentPtr<C>(m_id));
    }
    
    template <typename C>
    inline const C *ComponentHandle<C>::operator->() const
    {
        assert(valid());
        return m_manager->template getComponentPtr<C>(m_id);
    }
    
    template <typename C>
    C &ComponentHandle<C>::operator*()
    {
        assert(valid());
        return *m_manager->template getComponentPtr<C>(m_id);
    }
    
    template <typename C>
    const C &ComponentHandle<C>::operator*() const
    {
        assert(valid());
        return *m_manager->template getComponentPtr<C>(m_id);
    }
    
    template <typename C>
    C *ComponentHandle<C>::get()
    {
        assert(valid());
        return const_cast<C *>(m_manager->template getComponentPtr<C>(m_id));
    }
    
    template <typename C>
    const C *ComponentHandle<C>::get() const
    {
        assert(valid());
        return m_manager->template getComponentPtr<C>(m_id);
    }
    
    template <typename C>
    void ComponentHandle<C>::remove()
    {
        assert(valid());
        m_manager->template removeComponent<C>(m_id);
    }
    
    template <typename C>
    Object ComponentHandle<C>::object()
    {
        assert(valid());
        return m_manager->get(m_id);
    }
    
    template <typename C>
    bool ComponentHandle<C>::operator==(const ComponentHandle<C> &other) const
    {
        return m_manager == other.m_manager && m_id == other.m_id;
    }
    
    template <typename C>
    bool ComponentHandle<C>::operator!=(const ComponentHandle<C> &other) const
    {
        return !(*this == other);
    }

};

#endif
