#include "stdafx.h"
#include "base.hpp"
#include "ecs/ecs.hpp"
#include "system/envSys.hpp"
#include "system/textureSys.hpp"
#include "system/renderSys.hpp"
#include "system/shaderSys.hpp"
#include "system/loaderSys.hpp"
#include "system/bufferSys.hpp"
#include "system/materialSys.hpp"
#include "system/spatialSys.hpp"
#include "system/cameraSys.hpp"

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
    m_systemMgr.add<ShaderSystem>();
	m_systemMgr.add<RenderSystem>();
    m_systemMgr.add<MaterialSystem>();
	m_systemMgr.add<BufferSystem>();
    m_systemMgr.add<SpatialSystem>();
	m_systemMgr.add<CameraSystem>();
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
	while (1) {
		ecsObj.update(0.1f);
	}
	return 0;
}

