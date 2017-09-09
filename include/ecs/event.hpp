#ifndef RENDERER_ECS_EVENT_HPP
#define RENDERER_ECS_EVENT_HPP

#include "base.hpp"
#include "simplesignal.h"

namespace ecs
{

// argument: event pointer
// return: void
typedef Simple::Signal<void(const void *)> EventSignal;
typedef std::shared_ptr<EventSignal> EventSignalPtr;
typedef std::weak_ptr<EventSignal> EventSignalWeakPtr;
typedef size_t ConnectionID;

class BaseEvent
{
  public:
	typedef std::size_t TypeID;

  protected:
	static TypeID eventTypeCounter;
};

template <typename Derived>
class Event : public BaseEvent
{
  public:
	static TypeID typeID()
	{
		static TypeID id = ++eventTypeCounter;
		return id;
	}
};

class BaseReceiver
{
  public:
	virtual ~BaseReceiver()
	{
		for (auto connection : connections)
		{
			auto &ptr = connection.second.first;
			if (!ptr.expired())
			{
				ptr.lock()->disconnect(connection.second.second);
			}
		}
	}

	std::size_t getAliveSignals() const
	{
		std::size_t size = 0;
		for (auto connection : connections)
		{
			if (!connection.second.first.expired())
			{
				size++;
			}
		}
		return size;
	}

  private:
	friend class EventManager;
	std::unordered_map<BaseEvent::TypeID, std::pair<EventSignalWeakPtr, ConnectionID>> connections;
};

template <typename Derived>
class Receiver : public BaseReceiver
{
  public:
	virtual ~Receiver() {}
};

class EventManager
{
  public:
	EventManager() {}
	virtual ~EventManager() {}

	template <typename E, typename Receiver>
	void on(Receiver &receiver)
	{
		typedef void (Receiver::*receiveFunc)(const E &);
		receiveFunc receive = &Receiver::receive;
		auto sig = newSignal(Event<E>::typeID());
		ConnectionID connection = sig->connect([&](const void *evt) {
			receiver.receive(*(static_cast<const E *>(evt)));
		});
		BaseReceiver &base = receiver;
		base.connections.insert(std::make_pair(Event<E>::typeID(), std::make_pair(EventSignalWeakPtr(sig), connection)));
	}

	template <typename E, typename Receiver>
	void off(Receiver &receiver)
	{
		BaseReceiver &base = receiver;
		assert(base.connections.find(Event<E>::typeID()) != base.connections.end());
		auto pair = base.connections[Event<E>::typeID()];
		auto connection = pair.second;
		auto &ptr = pair.first;
		if (!ptr.expired())
		{
			ptr.lock()->disconnect(connection);
		}
		base.connections.erase(Event<E>::typeID());
	}

	template <typename E>
	void emit(const E &event)
	{
		auto sig = newSignal(Event<E>::typeID());
		sig->emit(&event);
	}

	template <typename E>
	void emit(std::unique_ptr<E> event)
	{
		auto sig = newSignal(Event<E>::typeID());
		sig->emit(event.get());
	}

	template <typename E, typename... Args>
	void emit(Args &&... args)
	{
		E event = E(std::forward<Args>(args)...);
		auto sig = newSignal(std::size_t(Event<E>::typeID()));
		sig->emit(&event);
	}

  private:
	EventSignalPtr &newSignal(std::size_t evtTypeID)
	{
		if (evtTypeID >= m_evtTypeID2Signal.size())
			m_evtTypeID2Signal.resize(evtTypeID + 1);
		if (!m_evtTypeID2Signal[evtTypeID])
			m_evtTypeID2Signal[evtTypeID] = std::make_shared<EventSignal>();
		return m_evtTypeID2Signal[evtTypeID];
	}

	std::vector<EventSignalPtr> m_evtTypeID2Signal;
};
};

#endif