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
	void update(ObjectManager &objs, EventManager &events, float dt) override
	{
		objs.each<Position, Direction>([dt](Object obj, Position &position, Direction &direction) {
			position.x += direction.x * dt;
			position.y += direction.y * dt;
		});
	};
};

struct Collision
{
	Collision(Object left, Object right) : left(left), right(right) {}

	Object left, right;
};

class CollisionSystem : public System<CollisionSystem>
{
  public:
	void update(ObjectManager &objs, EventManager &events, float dt) override
	{
		ComponentHandle<Position> left_position, right_position;
		for (Object left_entity : objs.entities_with_components(left_position))
		{
			for (Object right_entity : objs.entities_with_components(right_position))
			{
				if (1)
				{
					events.emit<Collision>(left_entity, right_entity);
				}
			}
		}
	};
};

struct DebugSystem : public System<DebugSystem>, public Receiver<DebugSystem>
{
	void configure(EventManager &events)
	{
		events.subscribe<Collision>(*this);
	}

	void update(ObjectManager &objs, EventManager &events, float dt) {}

	void receive(const Collision &collision)
	{
		std::cout << "entities collided: " << collision.left.ID() << " and " << collision.right.ID() << endl;
	}
};

class ECSDemo : public ECS
{
  public:
	ECSDemo()
	{
		systemMgr.add<MovementSystem>();
		systemMgr.configure();

		Object obj = objMgr.create();
		obj.assign<Position>(rand() % 100, rand() % 100);
		obj.assign<Direction>((rand() % 10) - 5, (rand() % 10) - 5);
	}

	void update(float dt)
	{
		systemMgr.update<MovementSystem>(dt);
	}
};

#endif