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
		ComponentHandle(const ObjectManager *manager, ObjectID id) : m_manager(manager), m_id(id) {}

		ComponentHandle() : m_manager(nullptr), m_id(0) {}

		bool valid() const;

		operator bool() const;

		C *operator->();

		const C *operator->() const;

		C &operator*();

		const C &operator*() const;

		C *get();

		const C *get() const;

		void remove();

		Object object();

		bool operator==(const ComponentHandle<C> &other) const;

		bool operator!=(const ComponentHandle<C> &other) const;

	  private:
		const ObjectManager *m_manager;
		ObjectID m_id;
	};
};

#endif
