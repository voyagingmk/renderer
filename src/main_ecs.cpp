#include "stdafx.h"
#include "base.hpp"
#include "ecs/ecs.hpp"

using namespace ecs; 

class MainECS: public ECS
{

public:
	MainECS();

	void setup();

	void update(float dt);
};

MainECS::MainECS() {

}

void MainECS::setup() {

}

void MainECS::update(float dt) {

}

int ecsMain(int argc, char *argv[])
{
	MainECS ecsObj;
	ecsObj.setup();
	ecsObj.update(0.1f);
	return 0;
}

