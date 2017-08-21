#ifndef RENDERER_ECS_EVENT_HPP
#define RENDERER_ECS_EVENT_HPP

#include "base.hpp"
#include "../simplesignal.h"

namespace ecs {

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

	/**
		* Event types should subclass from this.
		*
		* struct Explosion : public Event<Explosion> {
		*   Explosion(int damage) : damage(damage) {}
		*   int damage;
		* };
		*/
	template <typename Derived>
	class Event : public BaseEvent
	{
	  public:
		static TypeID typeID()
		{
			static TypeID id = eventTypeCounter++;
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

		std::size_t connected_signals() const
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

		/**
			* Subscribe an object to receive events of type E.
			*
			* Receivers must be subclasses of Receiver and must implement a receive() method accepting the given event type.
			*
			* eg.
			*
			*     struct ExplosionReceiver : public Receiver<ExplosionReceiver> {
			*       void receive(const Explosion &explosion) {
			*       }
			*     };
			*
			*     ExplosionReceiver receiver;
			*     em.subscribe<Explosion>(receiver);
			*/
		template <typename E, typename Receiver>
		void subscribe(Receiver &receiver)
		{
			typedef void (Receiver::*receiveFunc)(const E &);
			receiveFunc receive = &Receiver::receive;
			auto sig = signal_for(Event<E>::typeID());
			auto wrapper = EventCallbackWrapper<E>(std::bind(receive, &receiver, std::placeholders::_1));
			ConnectionID connection = sig->connect(wrapper);
			BaseReceiver &base = receiver;
			base.connections.insert(std::make_pair(Event<E>::typeID(), std::make_pair(EventSignalWeakPtr(sig), connection)));
		}

		/**
			* Unsubscribe an object in order to not receive events of type E anymore.
			*
			* Receivers must have subscribed for event E before unsubscribing from event E.
			*
			*/
		template <typename E, typename Receiver>
		void unsubscribe(Receiver &receiver)
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
			auto sig = signal_for(Event<E>::typeID());
			sig->emit(&event);
		}

		template <typename E>
		void emit(std::unique_ptr<E> event)
		{
			auto sig = signal_for(Event<E>::typeID());
			sig->emit(event.get());
		}

		// emit(0.1f, 10, "xxx")
		template <typename E, typename... Args>
		void emit(Args &&... args)
		{
			E event = E(std::forward<Args>(args)...);
			auto sig = signal_for(std::size_t(Event<E>::typeID()));
			sig->emit(&event);
		}

		std::size_t connected_receivers() const
		{
			std::size_t size = 0;
			for (EventSignalPtr handler : handlers_)
			{
				if (handler)
					size += handler->size();
			}
			return size;
		}

	  private:
		EventSignalPtr &signal_for(std::size_t id)
		{
			if (id >= handlers_.size())
				handlers_.resize(id + 1);
			if (!handlers_[id])
				handlers_[id] = std::make_shared<EventSignal>();
			return handlers_[id];
		}

		// Functor used as an event signal callback that casts to E.
		template <typename E>
		struct EventCallbackWrapper
		{
			explicit EventCallbackWrapper(std::function<void(const E &)> callback) : callback(callback) {}
			void operator()(const void *event) { callback(*(static_cast<const E *>(event))); }
			std::function<void(const E &)> callback;
		};

		std::vector<EventSignalPtr> handlers_;
	};

	};

#endif