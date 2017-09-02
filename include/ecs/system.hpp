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

	virtual void init(ObjectManager &objMgr, EventManager &evtMgr)
	{
		init(evtMgr);
	}

	virtual void init(EventManager &evtMgr) {}

	virtual void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) = 0;

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
	SystemManager(ObjectManager &objMgr, EventManager &evtMgr) : m_objMgr(objMgr),
																															 m_evtMgr(evtMgr) {}

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
		assert(m_inited && "SystemManager::init() not called");
		std::shared_ptr<S> s = system<S>();
		s->update(m_objMgr, m_evtMgr, dt);
	}

	void updateAll(float dt);

	void init();

private:
	bool m_inited = false;
	ObjectManager &m_objMgr;
	EventManager &m_evtMgr;
	std::unordered_map<BaseSystem::TypeID, std::shared_ptr<BaseSystem>> m_evtTypeID2System;
};
}

#endif