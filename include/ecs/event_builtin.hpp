#ifndef RENDERER_ECS_EVENT_BUILTIN_HPP
#define RENDERER_ECS_EVENT_BUILTIN_HPP

#include "event.hpp"
#include "component_handle.hpp"

namespace ecs
{
	class Object;

	struct ObjectCreatedEvent : public Event<ObjectCreatedEvent>
	{
		explicit ObjectCreatedEvent(Object* obj) : m_obj(obj) {}
		virtual ~ObjectCreatedEvent()
		{
		}
		Object* m_obj;
	};

	struct ObjectDestroyedEvent : public Event<ObjectDestroyedEvent>
	{
		explicit ObjectDestroyedEvent(ObjectID objID) : m_objID(objID) {}
		virtual ~ObjectDestroyedEvent() {}
		ObjectID m_objID;
	};

	template <typename C>
	struct ComponentAddedEvent : public Event<ComponentAddedEvent<C>>
	{
		ComponentAddedEvent(Object obj, ComponentHandle<C> component) : m_obj(obj), component(component) {}

		Object m_obj;
		ComponentHandle<C> component;
	};

	template <typename C>
	struct ComponentRemovedEvent : public Event<ComponentRemovedEvent<C>>
	{
		ComponentRemovedEvent(Object obj, ComponentHandle<C> component) : m_obj(obj), component(component) {}

		Object m_obj;
		ComponentHandle<C> component;
	};
};

#endif