#include "stdafx.h"
#include "test.hpp"

using namespace ecs;
using namespace std;

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
	obj.addComponent<Direction>(rand() % 10, rand() % 10);
	/*
	auto pos = obj.component<Position>();
	pos->x = 333;
	pos->y = 999;
	obj.removeComponent<Position>();
	auto dir = obj.component<Direction>();
	cout << "old " << dir->x << "," << dir->y << endl;
	obj.replace<Direction>(100, 999);
	cout << "new " << dir->x << "," << dir->y << endl;
	obj.removeComponent<Direction>();
	// cout << "after delete " << dir->x << "," << dir->y << endl;
	*/
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