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
#include "system/motionSys.hpp"
#include "system/configSys.hpp"

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
    m_systemMgr.add<MotionSystem>();
	m_systemMgr.add<LoaderSystem>();
	m_systemMgr.add<ConfigSystem>();
	m_systemMgr.init();
}

void MainECS::update(float dt) {
	m_systemMgr.updateAll(dt);
}

class TT {
public:
    TT() { printf("TT ctor \n"); }
    ~TT() { printf("TT dtor \n"); }
};




int ecsMain(int argc, char *argv[])
{
    QuaternionF p;
    
    p.FromEulerAngles(RadianF(0.0f), RadianF(0.0f), RadianF(0.0f));
    p.debug();
    p.FromEulerAngles(RadianF(90.0f), RadianF(0.0f), RadianF(0.0f));
    p.debug();
    p.FromEulerAngles(RadianF(0.0f), RadianF(90.0f), RadianF(0.0f));
    p.debug();
    p.FromEulerAngles(RadianF(0.0f), RadianF(0.0f), RadianF(90.0f));
    p.debug();
    
	MainECS ecsObj;
	ecsObj.setup();
	int interval = 10;
	int lastTick = SDL_GetTicks();
	while (1) {
		ecsObj.update(0.01f);
		while (SDL_GetTicks() < lastTick + interval) {
		}
		lastTick = SDL_GetTicks();
	}
	return 0;
}

