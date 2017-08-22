#ifndef RENDERER_ECS_SYSTEM_HPP
#define RENDERER_ECS_SYSTEM_HPP

#include "base.hpp"
#include "setting.hpp"
#include "object.hpp"
#include "event.hpp"

namespace ecs
{

class SystemManager;

/**
	* Base System class. Generally should not be directly used, instead see System<Derived>.
	*/
class BaseSystem
{
  public:
	typedef std::size_t TypeID;

	virtual ~BaseSystem();

	/**
		* Called once all Systems have been added to the SystemManager.
		*
		* Typically used to set up event handlers.
		*/
	virtual void configure(ObjectManager &entities, EventManager &events)
	{
		configure(events);
	}

	/**
		* Legacy configure(). Called by default implementation of configure(ObjectManager&, EventManager&).
		*/
	virtual void configure(EventManager &events) {}

	/**
		* Apply System behavior.
		*
		* Called every game step.
		*/
	virtual void update(ObjectManager &objs, EventManager &events, float dt) = 0;

	static TypeID m_SystemTypeCounter;

  protected:
};

/**
	* Use this class when implementing Systems.
	*
	* struct MovementSystem : public System<MovementSystem> {
	*   void update(ObjectManager &entities, EventManager &events, float dt) {
	*     // Do stuff to/with entities...
	*   }
	* }
	*/
template <typename Derived>
class System : public BaseSystem
{
  public:
	virtual ~System() {}

  private:
	friend class SystemManager;

	static TypeID typeID()
	{
		static TypeID typeID = m_SystemTypeCounter++;
		return typeID;
	}
};

class SystemManager
{
  public:
	SystemManager(ObjectManager &entity_manager,
				  EventManager &event_manager) : entity_manager_(entity_manager),
												 event_manager_(event_manager) {}

	/**
		* Add a System to the SystemManager.
		*
		* Must be called before Systems can be used.
		*
		* eg.
		* std::shared_ptr<MovementSystem> movement  -make_shared<MovementSystem>();
		* system.add(movement);
		*/
	template <typename S>
	void add(std::shared_ptr<S> system)
	{
		systems_.insert(std::make_pair(S::typeID(), system));
	}

	/**
		* Add a System to the SystemManager.
		*
		* Must be called before Systems can be used.
		*
		* eg.
		* auto movement = system.add<MovementSystem>();
		*/
	template <typename S, typename... Args>
	std::shared_ptr<S> add(Args &&... args)
	{
		std::shared_ptr<S> s(new S(std::forward<Args>(args)...));
		add(s);
		return s;
	}

	/**
		* Retrieve the registered System instance, if any.
		*
		*   std::shared_ptr<CollisionSystem> collisions = systems.system<CollisionSystem>();
		*
		* @return System instance or empty shared_std::shared_ptr<S>.
		*/
	template <typename S>
	std::shared_ptr<S> system()
	{
		auto it = systems_.find(S::typeID());
		assert(it != systems_.end());
		return it == systems_.end()
				   ? std::shared_ptr<S>()
				   : std::shared_ptr<S>(std::static_pointer_cast<S>(it->second));
	}

	/**
		* Call the System::update() method for a registered system.
		*/
	template <typename S>
	void update(float dt)
	{
		assert(initialized_ && "SystemManager::configure() not called");
		std::shared_ptr<S> s = system<S>();
		s->update(entity_manager_, event_manager_, dt);
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

	/**
		* Configure the system. Call after adding all Systems.
		*
		* This is typically used to set up event handlers.
		*/
	void configure();

  private:
	bool initialized_ = false;
	ObjectManager &entity_manager_;
	EventManager &event_manager_;
	std::unordered_map<BaseSystem::TypeID, std::shared_ptr<BaseSystem>> systems_;
};
}

#endif