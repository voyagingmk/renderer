#ifndef RENDERER_ECS_EVENT_BUILTIN_HPP
#define RENDERER_ECS_EVENT_BUILTIN_HPP

#include "obj.hpp"
#include "event.hpp"


namespace ecs
{
	struct EntityCreatedEvent : public Event<EntityCreatedEvent> {
		explicit EntityCreatedEvent(Object obj) : m_obj(obj) {}
		virtual ~EntityCreatedEvent() {
		}
		Object m_obj;
	};

	struct EntityDestroyedEvent : public Event<EntityDestroyedEvent> {
		explicit EntityDestroyedEvent(Object obj) : m_obj(obj) {}
		virtual ~EntityDestroyedEvent() {}
		Object m_obj;
	};

	template <typename C>
	struct ComponentAddedEvent : public Event<ComponentAddedEvent<C>> {
		ComponentAddedEvent(Object obj, ComponentHandle<C> component) :
			m_obj(obj), component(component) {}

		Object m_obj;
		ComponentHandle<C> component;
	};

	template <typename C>
	struct ComponentRemovedEvent : public Event<ComponentRemovedEvent<C>> {
		ComponentRemovedEvent(Object obj, ComponentHandle<C> component) :
			m_obj(obj), component(component) {}

		Object m_obj;
		ComponentHandle<C> component;
	};
};


#endif