#include "stdafx.h"
#include "test.hpp"

using namespace ecs;

ECSDemo::ECSDemo()
{
}

void ECSDemo::setup()
{
	m_systemMgr.add<MovementSystem>();
	m_systemMgr.add<DebugSystem>();

	m_systemMgr.configure();

	obj = m_objMgr.create();
	obj.addComponent<Position>(rand() % 100, rand() % 100);
	obj.addComponent<Direction>((rand() % 10) - 5, (rand() % 10) - 5);
}

void ECSDemo::update(float dt) const
{
	// m_systemMgr.update<MovementSystem>(dt);
	const ComponentHandle<Position> com = obj.component<Position>();
	printf("pos:%f,%f\n", com->x, com->y);
}