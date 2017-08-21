#ifndef RENDERER_ECS_HPP
#define RENDERER_ECS_HPP

#include "base.hpp"
#include "object.hpp"
#include "object_manager.hpp"
#include "event.hpp"
#include "system.hpp"

namespace ecs
{

class ECS
{
  public:
	ECS() : objMgr(eventMgr), systemMgr(objMgr, eventMgr) {}

	EventManager eventMgr;
	ObjectManager objMgr;
	SystemManager systemMgr;
};
};

#endif