#ifndef RENDERER_ECS_TEST_HPP
#define RENDERER_ECS_TEST_HPP

#include "base.hpp"
#include "ecs.hpp"

using namespace ecs;

struct Position
{
	Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

	float x, y;
};

struct Direction
{
	Direction(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

	float x, y;
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

struct Collision
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
	}

	void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {}

	void receive(const Collision &collision)
	{
		std::cout << "obj collided: " << collision.left.ID() << " and " << collision.right.ID() << std::endl;
	}

	void receive(const ObjectCreatedEvent &evt)
	{
		std::cout << "ObjectCreatedEvent: " << evt.m_obj->ID() << std::endl;
	}
};

class ECSDemo : public ECS
{
	Object obj;

  public:
	ECSDemo();

	void setup();

	void update(float dt) const;
};

#endif