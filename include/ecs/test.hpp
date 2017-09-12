#ifndef RENDERER_ECS_TEST_HPP
#define RENDERER_ECS_TEST_HPP

#include "base.hpp"
#include "ecs.hpp"

using namespace ecs;

struct Position
{
	Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
	~Position() {
		std::cout << "~Position()" << std::endl;
	}
	float x, y;
};

struct Direction
{
	Direction(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
	~Direction() {
		std::cout << "~Direction()" << std::endl;
	}
	float x, y;
};

struct ObjectColor
{
	ObjectColor(float r = 0.0f, float g = 0.0f, float b = 0.0f) : r(r), g(g), b(b) {}
	~ObjectColor() {
		std::cout << "~ObjectColor()" << std::endl;
	}
	float r, g, b;
};

class MovementSystem : public System<MovementSystem>
{
  public:
	void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override{
		/*
		objMgr.each<Position, Direction>([dt](Object obj, Position &position, Direction &direction) {
			position.x += direction.x * dt;
			position.y += direction.y * dt;
		});*/
	};
};

struct Collision: public Event<Collision>
{
  public:
	Collision(Object left, Object right) : left(left), right(right) {}

	Object left, right;
};

class CollisionSystem : public System<CollisionSystem>
{
  public:
	void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override
	{
		ComponentHandle<Position> left_position, right_position;
		/*
		for (Object left_obj : objMgr.entities_with_components(left_position))
		{
			for (Object right_obj : objMgr.entities_with_components(right_position))
			{
				if (1)
				{
					evtMgr.emit<Collision>(left_obj, right_obj);
				}
			}
		}*/
	};
};

struct DebugSystem : public System<DebugSystem>, public Receiver<DebugSystem>
{
  public:
	void init(EventManager &evtMgr)
	{
		evtMgr.on<Collision>(*this);
		evtMgr.on<ObjectCreatedEvent>(*this);
		evtMgr.on<ObjectDestroyedEvent>(*this);
		evtMgr.on<ComponentAddedEvent<Position>>(*this);
		evtMgr.on<ComponentRemovedEvent<Position>>(*this);
		evtMgr.on<ComponentAddedEvent<Direction>>(*this);
		evtMgr.on<ComponentRemovedEvent<Direction>>(*this);
		
	}

	void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {}

	void receive(const Collision &collision)
	{
		std::cout << "obj collided: " << collision.left.ID() << " and " << collision.right.ID() << std::endl;
	}

	void receive(const ObjectCreatedEvent &evt)
	{
		std::cout << "ObjectCreatedEvent: " << evt.m_obj.ID() << std::endl;
	}

	void receive(const ObjectDestroyedEvent &evt)
	{
		std::cout << "ObjectDestroyedEvent: " << evt.m_objID << std::endl;
	}

	void receive(const ComponentAddedEvent<Position> &evt)
	{
		std::cout << "ComponentAddedEvent Position: " << evt.m_obj.ID() << "," << evt.component->x << "," << evt.component->y << std::endl;
	}

	void receive(const ComponentRemovedEvent<Position> &evt)
	{
		std::cout << "ComponentRemovedEvent Position: " << evt.m_obj.ID() << "," << evt.component->x << "," << evt.component->y << std::endl;
	}

	void receive(const ComponentAddedEvent<Direction> &evt)
	{
		std::cout << "ComponentAddedEvent Direction: " << evt.m_obj.ID() << evt.component->x << "," << evt.component->y << std::endl;
	}

	void receive(const ComponentRemovedEvent<Direction> &evt)
	{
		std::cout << "ComponentRemovedEvent Direction: " << evt.m_obj.ID() << evt.component->x << "," << evt.component->y << std::endl;
	}
};

class ECSDemo : public ECS
{
	Object obj;

  public:
	ECSDemo();

	void setup();

	void update(float dt);
};

#endif