#include "stdafx.h"
#include "test.hpp"

ECSDemo::ECSDemo() {
	systemMgr.add<MovementSystem>();
	systemMgr.configure();

	Object obj = objMgr.create();
	obj.assign<Position>(rand() % 100, rand() % 100);
	obj.assign<Direction>((rand() % 10) - 5, (rand() % 10) - 5);
}