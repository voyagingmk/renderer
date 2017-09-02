#ifndef RENDERER_ECS_OBJECT_HPP
#define RENDERER_ECS_OBJECT_HPP

#include "setting.hpp"

namespace ecs
{
template <typename C>
class ComponentHandle;

class ObjectManager;

class Object
{
  public:
	Object() = default;
	Object(ObjectManager *manager, ObjectID id);
	Object(const Object &other) = default;
	Object &operator=(const Object &other) = default;

	bool valid() const;

	void destroy();

	void invalidate()
	{
		m_id = 0;
		m_manager = nullptr;
	}

	const ObjectID ID() const { return m_id; }

	bool operator==(const Object &other) const
	{
		return other.m_manager == m_manager && other.m_id == m_id;
	}

	bool operator!=(const Object &other) const
	{
		return !(other == *this);
	}

	bool operator<(const Object &other) const
	{
		return other.m_id < m_id;
	}

	template <typename C, typename... Args>
	ComponentHandle<C> addComponent(Args &&... args);

	template <typename C, typename... Args>
	ComponentHandle<C> replace(Args &&... args);

	template <typename C>
	void removeComponent();

	template <typename C>
	ComponentHandle<C> component() const;

	template <typename C>
	bool hasComponent() const;

  private:
	ObjectManager *m_manager = nullptr;
	ObjectID m_id;
};

template <typename C>
void Object::removeComponent()
{
	assert(valid() && hasComponent<C>());
	m_manager->removeComponent<C>(m_id);
}

template <typename C>
ComponentHandle<C> Object::component() const
{
	assert(valid());
	return m_manager->component<C>(m_id);
}

template <typename C, typename... Args>
ComponentHandle<C> Object::addComponent(Args &&... args)
{
	assert(valid());
	return m_manager->addComponent<C>(m_id, std::forward<Args>(args)...);
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
		handle = m_manager->addComponent<C>(m_id, std::forward<Args>(args)...);
	}
	return handle;
}

template <typename C>
bool Object::hasComponent() const
{
	assert(valid());
	return m_manager->hasComponent<C>(m_id);
}
};

#endif