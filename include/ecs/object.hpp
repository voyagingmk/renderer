#ifndef RENDERER_ECS_OBJ_HPP
#define RENDERER_ECS_OBJ_HPP

#include "base.hpp"
#include "setting.hpp"
#include "event.hpp"
#include "component_handle.hpp"
#include "event_builtin.hpp"
#include "MemoryPool.hpp"

namespace ecs {
	class ObjectManager;


	class Object
	{
	  public:
		Object() = default;
		Object(ObjectManager *manager, ObjectID id);
		Object(const Object &other) = default;
		Object &operator=(const Object &other) = default;

		bool valid() const;

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
		ComponentHandle<C> assign(Args &&... args)
		{
			assert(valid());
			assert(std::is_pod<C>::value == true);
			return m_manager->assign<C>(m_id, std::forward<Args>(args)...);
		}

		template <typename C>
		ComponentHandle<C> assign_from_copy(const C &component)
		{
			assert(valid());
			return m_manager->assign<C>(m_id, std::forward<const C &>(component));
		}

		template <typename C, typename... Args>
		ComponentHandle<C> replace(Args &&... args)
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
		void remove()
		{
			assert(valid() && has_component<C>());
			m_manager->remove<C>(m_id);
		}

		template <typename C, typename = typename std::enable_if<!std::is_const<C>::value>::type>
		ComponentHandle<C> component()
		{
			assert(valid());
			return m_manager->component<C>(m_id);
		}

		template <typename C, typename = typename std::enable_if<std::is_const<C>::value>::type>
		const ComponentHandle<C> component() const
		{
			assert(valid());
			return const_cast<const ObjectManager *>(m_manager)->component<const C>(m_id);
		}

		template <typename... Components>
		std::tuple<ComponentHandle<Components>...> components()
		{
			assert(valid());
			return m_manager->components<Components...>(m_id);
		}

		template <typename... Components>
		std::tuple<ComponentHandle<const Components>...> components() const
		{
			assert(valid());
			return const_cast<const OM *>(m_manager)->components<const Components...>(m_id);
		}

		template <typename C>
		bool has_component() const
		{
			assert(valid());
			return m_manager->has_component<C>(m_id);
		}

		/**
			* Destroy and invalidate this Object.
			*/
		void destroy();

	  private:
		ObjectManager *m_manager = nullptr;
		ObjectID m_id;
	};


};


#endif