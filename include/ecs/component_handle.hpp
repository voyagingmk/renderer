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
		ComponentHandle(ObjectManager *manager, ObjectID id) : m_manager(manager), m_id(id) {}

		ComponentHandle() : m_manager(nullptr), m_id(0) {}

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


	template <typename C>
	bool ComponentHandle<C>::valid() const
	{
		return m_manager && m_manager->valid(m_id) && m_manager->template has_component<C>(m_id);
	}

	template <typename C>
	ComponentHandle<C>::operator bool() const
	{
		return valid();
	}

	template <typename C>
	C* ComponentHandle<C>::operator->()
	{
		assert(valid());
		return m_manager->template get_component_ptr<C>(m_id);
	}

	template <typename C>
	const C* ComponentHandle<C>::operator->() const
	{
		assert(valid());
		return m_manager->template get_component_ptr<C>(m_id);
	}

	template <typename C>
	C& ComponentHandle<C>::operator*()
	{
		assert(valid());
		return *m_manager->template get_component_ptr<C>(m_id);
	}

	template <typename C>
	const C& ComponentHandle<C>::operator*() const
	{
		assert(valid());
		return *m_manager->template get_component_ptr<C>(m_id);
	}

	template <typename C>
	C* ComponentHandle<C>::get()
	{
		assert(valid());
		return m_manager->template get_component_ptr<C>(m_id);
	}

	template <typename C>
	const C* ComponentHandle<C>::get() const
	{
		assert(valid());
		return m_manager->template get_component_ptr<C>(m_id);
	}

	/**
	* Remove the component from its entity and destroy it.
	*/
	template <typename C>
	void ComponentHandle<C>::remove()
	{
		assert(valid());
		m_manager->template remove<C>(m_id);
	}

	template <typename C>
	Object ComponentHandle<C>::object()
	{
		assert(valid());
		return m_manager->get(m_id);
	}

	template <typename C>
	bool ComponentHandle<C>::operator==(const ComponentHandle<C> &other) const
	{
		return m_manager == other.m_manager && m_id == other.m_id;
	}

	template <typename C>
	bool ComponentHandle<C>::operator!=(const ComponentHandle<C> &other) const
	{
		return !(*this == other);
	}
};

#endif