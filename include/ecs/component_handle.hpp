#ifndef RENDERER_ECS_COMPONENT_HANDLE_HPP
#define RENDERER_ECS_COMPONENT_HANDLE_HPP

#include "setting.hpp"

namespace ecs
{

	class Object;
	class ObjectManager;

	template <typename C>
	class ComponentHandle
	{
	public:
		ComponentHandle(ObjectManager *manager, ObjectID id);
		ComponentHandle();

		bool valid() const;

		operator bool() const;

		C *operator->();

		const C *operator->() const;

		C &operator*();

		const C &operator*() const;

		C *get();

		const C *get() const;

		/**
			* Remove the component from its entity and destroy it.
			*/
		void remove();

		/**
			* Returns the Entity associated with the component
			*/
		Object object();

		bool operator==(const ComponentHandle<C> &other) const;

		bool operator!=(const ComponentHandle<C> &other) const;

	  private:
		ObjectManager *m_manager;
		ObjectID m_id;
	};
};

#endif