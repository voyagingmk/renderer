#include "stdafx.h"
#include "base.hpp"
#include "ecs/ecs.hpp"
#include "system/env.hpp"
#include "system/texture.hpp"
#include "system/render.hpp"
#include "system/loader.hpp"

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
    m_systemMgr.add<TextureSystem>();
	m_systemMgr.add<RenderSystem>();
	m_systemMgr.add<LoaderSystem>();
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

