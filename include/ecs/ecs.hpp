#ifndef RENDERER_ECS_HPP
#define RENDERER_ECS_HPP

#include "base.hpp"
#include "setting.hpp"
#include "object.hpp"
#include "component_handle.hpp"
#include "object_manager.hpp"
#include "event.hpp"
#include "event_builtin.hpp"
#include "system.hpp"
// implement

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