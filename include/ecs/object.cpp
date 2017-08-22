#include "stdafx.h"
#include "object.hpp"
#include "object_manager.hpp"
#include "component_handle.hpp"

namespace ecs {

	Object::Object(ObjectManager *manager, ObjectID id): 
		m_manager(manager), m_id(id)
	{}

	bool Object::valid() const
	{
		return m_manager && m_manager->valid(m_id);
	}


	void Object::destroy()
	{
		assert(valid());
		m_manager->destroy(m_id);
		invalidate();
	}		
	template <typename C>
	void Object::remove()
	{
		assert(valid() && has_component<C>());
		m_manager->remove<C>(m_id);
	}

	template <typename C, typename = typename std::enable_if<!std::is_const<C>::value>::type>
	ComponentHandle<C> Object::component()
	{
		assert(valid());
		return m_manager->component<C>(m_id);
	}

	template <typename C, typename = typename std::enable_if<std::is_const<C>::value>::type>
	const ComponentHandle<C> Object::component() const
	{
		assert(valid());
		return const_cast<const ObjectManager *>(m_manager)->component<const C>(m_id);
	}

	template <typename... Components>
	std::tuple<ComponentHandle<Components>...> Object::components()
	{
		assert(valid());
		return m_manager->components<Components...>(m_id);
	}

	template <typename... Components>
	std::tuple<ComponentHandle<const Components>...> Object::components() const 
	{
		assert(valid());
		return const_cast<const OM *>(m_manager)->components<const Components...>(m_id);
	}

	template <typename C, typename... Args>
	ComponentHandle<C> Object::assign(Args &&... args)
	{
		assert(valid());
		assert(std::is_pod<C>::value == true);
		return m_manager->assign<C>(m_id, std::forward<Args>(args)...);
	}

	template <typename C>
	ComponentHandle<C> Object::assign_from_copy(const C &component)
	{
		assert(valid());
		return m_manager->assign<C>(m_id, std::forward<const C &>(component));
	}

	template <typename C, typename... Args>
	ComponentHandle<C> Object::replace(Args &&... args)
	{
		assert(valid());
		auto handle = component<C>();
		if (handle)
		{
			*(handle.get()) = C(std::forward<Args>(args)...);
		}
		else
		{
			handle = m_manager->assign<C>(m_id, std::forward<Args>(args)...);
		}
		return handle;
	}

	template <typename C>
	bool Object::has_component() const
	{
		assert(valid());
		return m_manager->has_component<C>(m_id);
	}


};