#include "stdafx.h"
#include "test.hpp"

using namespace ecs; 

ECSDemo::ECSDemo() {

}

void ECSDemo::setup() 
{
	systemMgr.add<MovementSystem>();
	systemMgr.add<DebugSystem>();

	systemMgr.configure();

	obj = objMgr.create();
	obj.addComponent<Position>(rand() % 100, rand() % 100);
	obj.addComponent<Direction>((rand() % 10) - 5, (rand() % 10) - 5);
}

void ECSDemo::update(float dt) const
{
	// systemMgr.update<MovementSystem>(dt);
	const ComponentHandle<Position> com = obj.component<Position>();
	printf("pos:%f,%f\n", com.get()->x, com.get()->y);
}