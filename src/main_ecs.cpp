#include "stdafx.h"
#include "base.hpp"
#include "ecs/ecs.hpp"
#include "system/renderer.hpp"

using namespace ecs;
using namespace renderer;

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
	m_systemMgr.add<EnvSystem>();
	m_systemMgr.init();
}

void MainECS::update(float dt) {
	m_systemMgr.updateAll(dt);
}

int ecsMain(int argc, char *argv[])
{
	MainECS ecsObj;
	ecsObj.setup();
	ecsObj.update(0.1f);
	return 0;
}

