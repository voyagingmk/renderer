#ifndef RENDERER_ECS_OBJECT_MANAGER_HPP
#define RENDERER_ECS_OBJECT_MANAGER_HPP

#include "setting.hpp"
#include "utils/MemoryPool.hpp"
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
	typedef std::set<ObjectID> ObjectIDs;
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

	const Object get(ObjectID id) const;

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

	bool hasObjectIDs(ComponentTypeID typeID) {
		return typeID < m_objectIDs.size();
	}

	ObjectIDs& getObjectIDs(ComponentTypeID typeID) {
		// TODO 
		if (m_objectIDs.size() <= typeID) {
			m_objectIDs.resize(typeID + 1);
		}
		return m_objectIDs[typeID];
	}

	template <typename C>
	bool hasObjectIDs() {
		ComponentTypeID typeID = ComponentType<C>::typeID();
		return typeID < m_objectIDs.size();
	}

	template <typename C>
	ObjectIDs& getObjectIDs() {
		ComponentTypeID typeID = ComponentType<C>::typeID();
		if (m_objectIDs.size() <= typeID) {
			m_objectIDs.resize(typeID + 1);
		}
		return m_objectIDs[typeID];
	}

	template <typename C>
	ComponentHandle<C> getSingletonComponent() {
		ObjectIDs& objectIDs = getObjectIDs<C>();
		if (objectIDs.size() > 0) {
			Object obj = get(*(objectIDs.begin()));
			return obj.component<C>();
		}
		return ComponentHandle<C>();
	}

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

	template<typename... Coms>
	class BaseView {
	public:
		typedef std::tuple<Coms...> ComTuple;
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

			ViewIterator(ObjectManager *objMgr)
				: m_ObjMgr(objMgr) {
			}

			ViewIterator(ObjectManager *objMgr, ObjectIDs::iterator idx)
				: m_ObjMgr(objMgr), m_idx(idx) {
			}

			void next() {
				while ((m_idx != viewItEnd().m_idx) && !hasAllRequiredComs()) {
					++m_idx;
				}
			}

			template<std::size_t I = 0, typename FuncT>
			inline typename std::enable_if<I < std::tuple_size<ComTuple>::value, void>::type
			for_index(int index, FuncT f) {
				f(getTypeID<I>());
				if (index + 1 == getTypeCount()) {
					return;
				}
				for_index<(I+1)<std::tuple_size<ComTuple>::value?I+1:0, FuncT>(index + 1, f);
			}


			bool hasAllRequiredComs() {
				bool check = true;
				for_index(0, [&](ComponentTypeID typeID) {
					check &= m_ObjMgr->getComponentIdx(typeID, *m_idx) != -1;
				});
				return check;
			}


			const ViewIterator viewItBegin() const { return ViewIterator(m_ObjMgr, m_ObjMgr->getObjectIDs(getTypeID<0>()).begin()); }
			const ViewIterator viewItEnd() const { return ViewIterator(m_ObjMgr, m_ObjMgr->getObjectIDs(getTypeID<0>()).end()); }

			template<size_t Idx>
			const ComponentTypeID getTypeID() const {
				using C = typename std::tuple_element<Idx, ComTuple>::type;
				return ComponentType<C>::typeID();
			}

			inline const size_t getTypeCount() const {
				return std::tuple_size<ComTuple>::value;
			}

		protected:
			ObjectManager *m_ObjMgr;
			ObjectIDs::iterator m_idx;
		};

		ViewIterator begin() { return ViewIterator(m_ObjMgr).viewItBegin(); }
		ViewIterator end() { return ViewIterator(m_ObjMgr).viewItEnd(); }
		const ViewIterator begin() const { return ViewIterator(m_ObjMgr).viewItBegin(); }
		const ViewIterator end() const { return ViewIterator(m_ObjMgr).viewItEnd(); }

	private:
		friend class ObjectManager;

		BaseView(ObjectManager *manager) :
			m_ObjMgr(manager) {}

		ObjectManager *m_ObjMgr;
	};


	template<typename... Coms>
	BaseView<Coms...> entities() {
		return BaseView<Coms...>(this);
	}

  private:

	friend class Object;

	template <typename C>
	friend class ComponentHandle;

	template <typename C>
	size_t getComponentIdx(ObjectID id) const;

	size_t getComponentIdx(ComponentTypeID typeID, ObjectID id) const;

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
	std::vector<ComponentHash> m_comHashes; // m_comHashes[objectID]
	std::vector<ObjectIDs> m_objectIDs; // m_objectIDs[comTypeID]

	struct ComponentMetaInfo
	{
		MemoryPoolBase *pool = nullptr;
	};

	std::map<ComponentTypeID, ComponentMetaInfo> m_comMetaInfo;

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
            
    template <typename C>
    ComponentHandle<C> Object::getSingletonComponent() const
    {
        assert(valid());
        return m_manager->getSingletonComponent<C>();
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
	ObjectIDs& objectIDs = getObjectIDs(typeID);
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
	ObjectIDs& objectIDs = getObjectIDs(typeID);
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
        return const_cast<C&>(*m_manager->template getComponentPtr<C>(m_id));
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
	const Object ComponentHandle<C>::object() const
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
