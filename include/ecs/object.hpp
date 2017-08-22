#ifndef RENDERER_ECS_OBJ_HPP
#define RENDERER_ECS_OBJ_HPP

#include "setting.hpp"

namespace ecs {
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
		ComponentHandle<C> assign(Args &&... args);

		template <typename C>
		ComponentHandle<C> assign_from_copy(const C &component);


		template <typename C, typename... Args>
		ComponentHandle<C> replace(Args &&... args);

		template <typename C>
		void remove();

		template <typename C, typename = typename std::enable_if<!std::is_const<C>::value>::type>
		ComponentHandle<C> component();

		template <typename C, typename = typename std::enable_if<std::is_const<C>::value>::type>
		const ComponentHandle<C> component() const;

		template <typename... Components>
		std::tuple<ComponentHandle<Components>...> components();

		template <typename... Components>
		std::tuple<ComponentHandle<const Components>...> components() const;

		template <typename C>
		bool has_component() const;

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