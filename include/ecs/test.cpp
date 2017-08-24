#include "stdafx.h"
#include "test.hpp"

using namespace ecs; 

ECSDemo::ECSDemo()
{
	systemMgr.add<MovementSystem>();
	systemMgr.add<DebugSystem>();

	systemMgr.configure();

	Object obj = objMgr.create();
	obj.addComponent<Position>(rand() % 100, rand() % 100);
	obj.addComponent<Direction>((rand() % 10) - 5, (rand() % 10) - 5);
	auto com = obj.component<Position>();
	printf("pos:%f,%f\n", com.get()->x, com.get()->y);
}

void ECSDemo::update(float dt)
{
	systemMgr.update<MovementSystem>(dt);
}