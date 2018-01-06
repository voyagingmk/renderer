#include "stdafx.h"
#include "base.hpp"
#include "ecs/ecs.hpp"
#include "system/envSys.hpp"
#include "system/textureSys.hpp"
#include "system/renderSys.hpp"
#include "system/shaderSys.hpp"
#include "system/loaderSys.hpp"
#include "system/physicsSys.hpp"
#include "system/bufferSys.hpp"
#include "system/materialSys.hpp"
#include "system/spatialSys.hpp"
#include "system/animationSys.hpp"
#include "system/cameraSys.hpp"
#include "system/motionSys.hpp"
#include "system/configSys.hpp"
#include "system/uiSys.hpp"
#include "system/lightSys.hpp"
#include "system/batchSys.hpp"
#include "system/statSys.hpp"
#include "system/sceneGraphSys.hpp"
#include "system/bvhSys.hpp"
#include "system/kdtreeSys.hpp"
#include "utils/parallel.hpp"



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
	m_systemMgr.add<UISystem>();
	m_systemMgr.add<StatSystem>();
    m_systemMgr.add<EnvSystem>();
	m_systemMgr.add<BatchSystem>();
	m_systemMgr.add<BVHSystem>();
	m_systemMgr.add<KdTreeSystem>();
    m_systemMgr.add<PhysicsSystem>();
	m_systemMgr.add<SceneGraphSystem>();
	m_systemMgr.add<LightSystem>();
    m_systemMgr.add<TextureSystem>();
    m_systemMgr.add<ShaderSystem>();
	m_systemMgr.add<RenderSystem>();
    m_systemMgr.add<MaterialSystem>();
	m_systemMgr.add<BufferSystem>();
    m_systemMgr.add<SpatialSystem>();
    m_systemMgr.add<CameraSystem>();
    m_systemMgr.add<MotionSystem>();
    m_systemMgr.add<AnimationSystem>();
	m_systemMgr.add<LoaderSystem>();
	m_systemMgr.add<ConfigSystem>();
	m_systemMgr.init();
}

void MainECS::update(float dt) {
	m_systemMgr.updateAll(dt);
}



int ecsMain(int argc, char *argv[])
{
	std::cout << "default max_align_t:" << alignof(std::max_align_t) << std::endl;
	MainECS ecsObj;
	ecsObj.setup();
	const uint32_t intervalMS = 1000 / EcsUpdateFPS;
	uint32_t lastTick = SDL_GetTicks(); // 0
    auto com = ecsObj.m_objMgr.getSingletonComponent<SDLContext>();
	while (!com->shouldExit) {
		while (SDL_GetTicks() < lastTick + intervalMS) {
		}
	    uint32_t nowTick = SDL_GetTicks();
        ecsObj.update(0.001f * (nowTick - lastTick));
        lastTick = nowTick;
	}
    TerminateWorkerThreads();
	return 0;
}

