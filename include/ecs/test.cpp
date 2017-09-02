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

	m_systemMgr.init();

	obj = m_objMgr.create();
	obj.addComponent<Position>(rand() % 100, rand() % 100);
	obj.addComponent<Direction>((rand() % 10) - 5, (rand() % 10) - 5);
	obj.destroy();
}

void ECSDemo::update(float dt)
{
	m_systemMgr.updateAll(dt);
	if (obj.valid()) {
		const ComponentHandle<Position> com = obj.component<Position>();
		printf("pos:%f,%f\n", com->x, com->y);
	}
}