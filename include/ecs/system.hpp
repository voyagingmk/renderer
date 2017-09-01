#ifndef RENDERER_ECS_SYSTEM_HPP
#define RENDERER_ECS_SYSTEM_HPP

#include "base.hpp"
#include "setting.hpp"
#include "object.hpp"
#include "event.hpp"

namespace ecs
{

class SystemManager;

class BaseSystem
{
public:
	typedef std::size_t TypeID;

	virtual ~BaseSystem();

	virtual void configure(ObjectManager &entities, EventManager &events)
	{
		configure(events);
	}

	virtual void configure(EventManager &events) {}

	virtual void update(ObjectManager &objs, EventManager &events, float dt) = 0;

	static TypeID m_SystemTypeCounter;

protected:
};

template <typename Derived>
class System : public BaseSystem
{
public:
	virtual ~System() {}

private:
	friend class SystemManager;

	static TypeID typeID()
	{
		static TypeID typeID = ++m_SystemTypeCounter;
		return typeID;
	}
};

class SystemManager
{
public:
	SystemManager(ObjectManager &entity_manager,
								EventManager &event_manager) : objMgr(entity_manager),
																							 evtMgr(event_manager) {}

	template <typename S>
	void add(std::shared_ptr<S> system)
	{
		m_evtTypeID2System.insert(std::make_pair(S::typeID(), system));
	}

	template <typename S, typename... Args>
	std::shared_ptr<S> add(Args &&... args)
	{
		std::shared_ptr<S> s(new S(std::forward<Args>(args)...));
		add(s);
		return s;
	}

	template <typename S>
	std::shared_ptr<S> get()
	{
		auto it = m_evtTypeID2System.find(S::typeID());
		return it == m_evtTypeID2System.end() ? nullptr : std::shared_ptr<S>(std::static_pointer_cast<S>(it->second));
	}

	template <typename S>
	void update(float dt)
	{
		assert(m_inited && "SystemManager::configure() not called");
		std::shared_ptr<S> s = system<S>();
		s->update(objMgr, evtMgr, dt);
	}

	/**
		* Call System::update() on all registered systems.
		*
		* The order which the registered systems are updated is arbitrary but consistent,
		* meaning the order which they will be updated cannot be specified, but that order
		* will stay the same as long no systems are added or removed.
		*
		* If the order in which systems update is important, use SystemManager::update()
		* to manually specify the update order. EntityX does not yet support a way of
		* specifying priority for update_all().
		*/
	void update_all(float dt);

	void configure();

private:
	bool m_inited = false;
	ObjectManager &objMgr;
	EventManager &evtMgr;
	std::unordered_map<BaseSystem::TypeID, std::shared_ptr<BaseSystem>> m_evtTypeID2System;
};
}

#endif