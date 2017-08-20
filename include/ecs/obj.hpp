#ifndef RENDERER_ECS_MANAGER_HPP
#define RENDERER_ECS_MANAGER_HPP

#include "base.hpp"
#include "setting.hpp"
#include "event.hpp"
#include "MemoryPool.hpp"

namespace ecs
{
typedef uint64_t ObjectID;
class ObjectManager;
class EntityCreatedEvent;
class EntityDestroyedEvent;
class ComponentAddedEvent;
class ComponentRemovedEvent;

template <typename C, typename EM = ObjectManager>
class ComponentHandle;

class Object
{
  public:
	Object() = default;
	Object(ObjectManager *manager, ObjectID id) : m_manager(manager), m_id(id) {}
	Object(const Object &other) = default;
	Object &operator=(const Object &other) = default;
	
	inline bool valid() const {
		return m_manager && m_manager->valid(m_id);
	}

	void invalidate() {
		m_id = 0;
		m_manager = nullptr;
	}

	const ObjectID ID() const { return m_id; }

	bool operator==(const Object &other) const {
		return other.m_manager == m_manager && other.m_id == m_id;
	}

	bool operator!=(const Object &other) const {
		return !(other == *this);
	}

	bool operator<(const Object &other) const {
		return other.m_id < m_id;
	}

	template <typename C, typename... Args>
	ComponentHandle<C> assign(Args &&... args) {
		assert(valid());
		return m_manager->assign<C>(m_id, std::forward<Args>(args)...);
	}

	template <typename C>
	ComponentHandle<C> assign_from_copy(const C &component) {
		assert(valid());
		return m_manager->assign<C>(m_id, std::forward<const C &>(component));
	}

	template <typename C, typename... Args>
	ComponentHandle<C> replace(Args &&... args) {
		assert(valid());
		auto handle = component<C>();
		if (handle) {
			*(handle.get()) = C(std::forward<Args>(args) ...);
		}
		else {
			handle = m_manager->assign<C>(m_id, std::forward<Args>(args) ...);
		}
		return handle;
	}

	template <typename C>
	void remove() {
		assert(valid() && has_component<C>());
		m_manager->remove<C>(m_id);
	}

	template <typename C, typename = typename std::enable_if<!std::is_const<C>::value>::type>
	ComponentHandle<C> component() {
		assert(valid());
		return m_manager->component<C>(m_id);
	}


	template <typename C, typename = typename std::enable_if<std::is_const<C>::value>::type>
	const ComponentHandle<C, const ObjectManager> component() const {
		assert(valid());
		return const_cast<const EntityManager*>(m_manager)->component<const C>(m_id);
	}

	template <typename... Components>
	std::tuple<ComponentHandle<Components>...> components() {
		assert(valid());
		return m_manager->components<Components...>(m_id);
	}

	template <typename... Components>
	std::tuple<ComponentHandle<const Components, const ObjectManager>...> components() const {
		assert(valid());
		return const_cast<const EntityManager*>(m_manager)->components<const Components...>(m_id);
	}

	template <typename C>
	bool has_component() const {
		assert(valid());
		return m_manager->has_component<C>(m_id);
	}

	template <typename A, typename... Args>
	void unpack(ComponentHandle<A> &a, ComponentHandle<Args> &... args) {
		assert(valid());
		m_manager->unpack(m_id, a, args ...);
	}


	/**
		* Destroy and invalidate this Object.
		*/
	void destroy() {
		assert(valid());
		m_manager->destroy(m_id);
		invalidate();
	}

	std::bitset<MAX_COMPONENTS> component_mask() const {
		return m_manager->component_mask(m_id);
	}

  private:
	ObjectManager *m_manager = nullptr;
	ObjectID m_id;
};

class ObjectManager
{
  public:
	typedef std::bitset<MAX_COMPONENTS> ComponentMask;

	explicit ObjectManager(EventManager &event_manager) : m_event_manager(event_manager) { }
	
	virtual ~ObjectManager() {
		reset();
	}

	/// An iterator over a view of the entities in an ObjectManager.
	/// If All is true it will iterate over all valid entities and will ignore the entity mask.
	template <class Delegate, bool All = false>
	class ViewIterator : public std::iterator<std::input_iterator_tag, ObjectID>
	{
	  public:
		Delegate &operator++()
		{
			++i_;
			next();
			return *static_cast<Delegate *>(this);
		}
		bool operator==(const Delegate &rhs) const { return i_ == rhs.i_; }
		bool operator!=(const Delegate &rhs) const { return i_ != rhs.i_; }
		Object operator*() { return Object(m_manager, m_manager->create_id(i_)); }
		const Object operator*() const { return Object(m_manager, m_manager->create_id(i_)); }

	  protected:
		ViewIterator(ObjectManager *manager, uint32_t index)
			: m_manager(manager), i_(index), capacity_(m_manager->capacity()), free_cursor_(~0UL)
		{
			if (All)
			{
				std::sort(m_manager->free_list.begin(), m_manager->free_list.end());
				free_cursor_ = 0;
			}
		}
		ViewIterator(ObjectManager *manager, const ComponentMask mask, uint32_t index)
			: m_manager(manager), mask_(mask), i_(index), capacity_(m_manager->capacity()), free_cursor_(~0UL)
		{
			if (All)
			{
				std::sort(m_manager->free_list.begin(), m_manager->free_list.end());
				free_cursor_ = 0;
			}
		}

		void next()
		{
			while (i_ < capacity_ && !predicate())
			{
				++i_;
			}

			if (i_ < capacity_)
			{
				Object entity = m_manager->get(m_manager->create_id(i_));
				static_cast<Delegate *>(this)->next_entity(entity);
			}
		}

		inline bool valid_entity()
		{
			const std::vector<uint32_t> &free_list = m_manager->free_list;
			if (free_cursor_ < free_list.size() && free_list[free_cursor_] == i_)
			{
				++free_cursor_;
				return false;
			}
			return true;
		}

		ObjectManager *m_manager;
		ComponentMask mask_;
		uint32_t i_;
		size_t capacity_;
		size_t free_cursor_;
	};

	template <bool All>
	class BaseView
	{
	  public:
		class Iterator : public ViewIterator<Iterator, All>
		{
		  public:
			Iterator(ObjectManager *manager,
					 const ComponentMask mask,
					 uint32_t index) : ViewIterator<Iterator, All>(manager, mask, index)
			{
				ViewIterator<Iterator, All>::next();
			}

			void next_entity(Object &entity) {}
		};

		Iterator begin() { return Iterator(m_manager, mask_, 0); }
		Iterator end() { return Iterator(m_manager, mask_, uint32_t(m_manager->capacity())); }
		const Iterator begin() const { return Iterator(m_manager, mask_, 0); }
		const Iterator end() const { return Iterator(m_manager, mask_, m_manager->capacity()); }

	  private:
		friend class ObjectManager;

		explicit BaseView(ObjectManager *manager) : m_manager(manager) { mask_.set(); }
		BaseView(ObjectManager *manager, ComponentMask mask) : m_manager(manager), mask_(mask) {}

		ObjectManager *m_manager;
		ComponentMask mask_;
	};

	template <bool All, typename... Components>
	class TypedView : public BaseView<All>
	{
	  public:
		template <typename T>
		struct identity
		{
			typedef T type;
		};

		void each(typename identity<std::function<void(Object entity, Components &...)>>::type f)
		{
			for (auto it : *this)
				f(it, *(it.template component<Components>().get())...);
		}

	  private:
		friend class ObjectManager;

		explicit TypedView(ObjectManager *manager) : BaseView<All>(manager) {}
		TypedView(ObjectManager *manager, ComponentMask mask) : BaseView<All>(manager, mask) {}
	};

	template <typename... Components>
	using View = TypedView<false, Components...>;
	typedef BaseView<true> DebugView;

	template <typename... Components>
	class UnpackingView
	{
	  public:
		struct Unpacker
		{
			explicit Unpacker(ComponentHandle<Components> &... handles) : handles(std::tuple<ComponentHandle<Components> &...>(handles...)) {}

			void unpack(entityx::Object &entity) const
			{
				unpack_<0, Components...>(entity);
			}

		  private:
			template <int N, typename C>
			void unpack_(entityx::Object &entity) const
			{
				std::get<N>(handles) = entity.component<C>();
			}

			template <int N, typename C0, typename C1, typename... Cn>
			void unpack_(entityx::Object &entity) const
			{
				std::get<N>(handles) = entity.component<C0>();
				unpack_<N + 1, C1, Cn...>(entity);
			}

			std::tuple<ComponentHandle<Components> &...> handles;
		};

		class Iterator : public ViewIterator<Iterator>
		{
		  public:
			Iterator(ObjectManager *manager,
					 const ComponentMask mask,
					 uint32_t index,
					 const Unpacker &unpacker) : ViewIterator<Iterator>(manager, mask, index), unpacker_(unpacker)
			{
				ViewIterator<Iterator>::next();
			}

			void next_entity(Object &entity)
			{
				unpacker_.unpack(entity);
			}

		  private:
			const Unpacker &unpacker_;
		};

		Iterator begin() { return Iterator(m_manager, mask_, 0, unpacker_); }
		Iterator end() { return Iterator(m_manager, mask_, static_cast<uint32_t>(m_manager->capacity()), unpacker_); }
		const Iterator begin() const { return Iterator(m_manager, mask_, 0, unpacker_); }
		const Iterator end() const { return Iterator(m_manager, mask_, static_cast<uint32_t>(m_manager->capacity()), unpacker_); }

	  private:
		friend class ObjectManager;

		UnpackingView(ObjectManager *manager, ComponentMask mask, ComponentHandle<Components> &... handles) : m_manager(manager), mask_(mask), unpacker_(handles...) {}

		ObjectManager *m_manager;
		ComponentMask mask_;
		Unpacker unpacker_;
	};

	/**
		* Number of managed entities.
		*/
	size_t size() const { return 0; }

	/**
		* Current entity capacity.
		*/
	size_t capacity() const { return 0; }

	bool valid(ObjectID id) const {
		return id > 0;
	}
	Object create()
	{
		ObjectID id;
		if (free_list.empty())
		{
			id = m_objectIDCounter++;
		}
		else
		{
			id = free_list.back();
			free_list.pop_back();
		}
		Object entity(this, id);
		m_event_manager.emit<EntityCreatedEvent>(entity);
		return entity;
	}

	/**
		* Create a new Object by copying another. Copy-constructs each component.
		*
		* Emits EntityCreatedEvent.
		*/
	Object create_from_copy(Object original)
	{
		assert(original.valid());
		auto clone = create();
		auto mask = original.component_mask();
		for (size_t i = 0; i < component_helpers_.size(); i++)
		{
			BaseComponentHelper *helper = component_helpers_[i];
			if (helper && mask.test(i))
				helper->copy_component_to(original, clone);
		}
		return clone;
	}

	/**
		* Destroy an existing ObjectID and its associated Components.
		*
		* Emits EntityDestroyedEvent.
		*/
	void destroy(ObjectID id)
	{
		for (size_t i = 0; i < component_helpers_.size(); i++)
		{
			BaseComponentHelper *helper = component_helpers_[i];
			if (helper && mask.test(i))
				helper->remove_component(id);
			e.remove<C>();
		}
		m_event_manager.emit<EntityDestroyedEvent>(id);
		free_list.push_back(id);
	}

	Object get(ObjectID id)
	{
		assert_valid(id);
		return Object(this, id);
	}

	/**
		* Create an ObjectID for a slot.
		*
		* NOTE: Does *not* check for validity, but the ObjectID constructor will
		* fail if the ID is invalid.
		*/
	ObjectID create_id(uint32_t index) const
	{
		return ObjectID(index, entity_version_[index]);
	}

	/**
		* Assign a Component to an ObjectID, passing through Component constructor arguments.
		*
		*     Position &position = em.assign<Position>(e, x, y);
		*
		* @returns Smart pointer to newly created component.
		*/
	template <typename C, typename... Args>
	ComponentHandle<C> assign(ObjectID id, Args &&... args)
	{
		assert_valid(id);
		const BaseComponent::Family family = component_family<C>();

		// Placement new into the component pool.
		MemoryPool<C> *pool = getComponentPool<C>();
		pool->newElement(id, std::forward<Args>(args)...);

		// Create and return handle.
		ComponentHandle<C> component(this, id);
		m_event_manager.emit<ComponentAddedEvent<C>>(Object(this, id), component);
		return component;
	}

	template <typename C>
	MemoryPool<C>* getComponentPool() {
		static MemoryPool<C> pool;
		return &pool;
	}

	/**
		* Remove a Component from an ObjectID
		*
		* Emits a ComponentRemovedEvent<C> event.
		*/
	template <typename C>
	void remove(ObjectID id)
	{
		assert_valid(id);
		const BaseComponent::Family family = component_family<C>();
		const uint32_t index = id.index();

		MemoryPool<C>* pool = getComponentPool<C>();
		ComponentHandle<C> component(this, id);
		m_event_manager.emit<ComponentRemovedEvent<C>>(Object(this, id), component);

		// Call destructor.
		pool->destroy(id);
	}

	/**
		* Check if an Object has a component.
		*/
	template <typename C>
	bool has_component(ObjectID id) const
	{
		assert_valid(id);
		size_t family = component_family<C>();
		// We don't bother checking the component mask, as we return a nullptr anyway.
		if (family >= component_pools_.size())
			return false;
		MemoryPool *pool = component_pools_[family];
		if (!pool)
			return false;
		return true;
	}

	/**
		* Retrieve a Component assigned to an ObjectID.
		*
		* @returns Pointer to an instance of C, or nullptr if the ObjectID does not have that Component.
		*/
	template <typename C, typename = typename std::enable_if<!std::is_const<C>::value>::type>
	ComponentHandle<C> component(ObjectID id)
	{
		assert_valid(id);
		size_t family = component_family<C>();
		// We don't bother checking the component mask, as we return a nullptr anyway.
		if (family >= component_pools_.size())
			return ComponentHandle<C>();
		MemoryPool *pool = component_pools_[family];
		if (!pool )
			return ComponentHandle<C>();
		return ComponentHandle<C>(this, id);
	}

	/**
		* Retrieve a Component assigned to an ObjectID.
		*
		* @returns Component instance, or nullptr if the ObjectID does not have that Component.
		*/
	template <typename C, typename = typename std::enable_if<std::is_const<C>::value>::type>
	const ComponentHandle<C, const ObjectManager> component(ObjectID id) const
	{
		assert_valid(id);
		size_t family = component_family<C>();
		// We don't bother checking the component mask, as we return a nullptr anyway.
		if (family >= component_pools_.size())
			return ComponentHandle<C, const ObjectManager>();
		MemoryPool *pool = component_pools_[family];
		if (!pool)
			return ComponentHandle<C, const ObjectManager>();
		return ComponentHandle<C, const ObjectManager>(this, id);
	}

	template <typename... Components>
	std::tuple<ComponentHandle<Components>...> components(ObjectID id)
	{
		return std::make_tuple(component<Components>(id)...);
	}

	template <typename... Components>
	std::tuple<ComponentHandle<const Components, const ObjectManager>...> components(ObjectID id) const
	{
		return std::make_tuple(component<const Components>(id)...);
	}

	/**
		* Find Entities that have all of the specified Components.
		*
		* @code
		* for (Object entity : entity_manager.entities_with_components<Position, Direction>()) {
		*   ComponentHandle<Position> position = entity.component<Position>();
		*   ComponentHandle<Direction> direction = entity.component<Direction>();
		*
		*   ...
		* }
		* @endcode
		*/
	template <typename... Components>
	View<Components...> entities_with_components()
	{
		auto mask = component_mask<Components...>();
		return View<Components...>(this, mask);
	}

	template <typename T>
	struct identity
	{
		typedef T type;
	};

	template <typename... Components>
	void each(typename identity<std::function<void(Object entity, Components &...)>>::type f)
	{
		return entities_with_components<Components...>().each(f);
	}

	/**
		* Find Entities that have all of the specified Components and assign them
		* to the given parameters.
		*
		* @code
		* ComponentHandle<Position> position;
		* ComponentHandle<Direction> direction;
		* for (Object entity : entity_manager.entities_with_components(position, direction)) {
		*   // Use position and component here.
		* }
		* @endcode
		*/
	template <typename... Components>
	UnpackingView<Components...> entities_with_components(ComponentHandle<Components> &... components)
	{
		auto mask = component_mask<Components...>();
		return UnpackingView<Components...>(this, mask, components...);
	}

	/**
		* Iterate over all *valid* entities (ie. not in the free list). Not fast,
		* so should only be used for debugging.
		*
		* @code
		* for (Object entity : entity_manager.entities_for_debugging()) {}
		*
		* @return An iterator view over all valid entities.
		*/
	DebugView entities_for_debugging()
	{
		return DebugView(this);
	}

	template <typename C>
	void unpack(ObjectID id, ComponentHandle<C> &a)
	{
		assert_valid(id);
		a = component<C>(id);
	}

	/**
		* Unpack components directly into pointers.
		*
		* Components missing from the entity will be set to nullptr.
		*
		* Useful for fast bulk iterations.
		*
		* ComponentHandle<Position> p;
		* ComponentHandle<Direction> d;
		* unpack<Position, Direction>(e, p, d);
		*/
	template <typename A, typename... Args>
	void unpack(ObjectID id, ComponentHandle<A> &a, ComponentHandle<Args> &... args)
	{
		assert_valid(id);
		a = component<A>(id);
		unpack<Args...>(id, args...);
	}

	/**
		* Destroy all entities and reset the ObjectManager.
		*/
	void reset();

	// Retrieve the component family for a type.
	template <typename C>
	static BaseComponent::Family component_family()
	{
		return Component<typename std::remove_const<C>::type>::family();
	}

  private:
	friend class Object;
	template <typename C, typename EM>
	friend class ComponentHandle;

	template <typename C>
	C *get_component_ptr(ObjectID id)
	{
		assert(valid(id));
		MemoryPool *pool = component_pools_[component_family<C>()];
		assert(pool);
		return static_cast<C *>(pool->get(id.index()));
	}

	template <typename C>
	const C *get_component_ptr(ObjectID id) const
	{
		assert_valid(id);
		MemoryPool *pool = component_pools_[component_family<C>()];
		assert(pool);
		return static_cast<const C *>(pool->get(id.index()));
	}

	template <typename C>
	ComponentMask component_mask()
	{
		ComponentMask mask;
		mask.set(component_family<C>());
		return mask;
	}

	template <typename C1, typename C2, typename... Components>
	ComponentMask component_mask()
	{
		return component_mask<C1>() | component_mask<C2, Components...>();
	}

	template <typename C>
	ComponentMask component_mask(const ComponentHandle<C> &c)
	{
		return component_mask<C>();
	}

	template <typename C1, typename... Components>
	ComponentMask component_mask(const ComponentHandle<C1> &c1, const ComponentHandle<Components> &... args)
	{
		return component_mask<C1, Components...>();
	}


	uint32_t m_objectIDCounter = 0;

	EventManager& m_event_manager;
	// Each element in component_helpers_ corresponds to a ComponentHelper for a Component type.
	// The index into the vector is the Component::family().
	std::vector<BaseComponentHelper *> component_helpers_;
	// Vector of entity version numbers. Incremented each time an entity is destroyed
	std::vector<uint32_t> entity_version_;
	// List of available entity slots.
	std::vector<uint32_t> free_list;
};
};

#endif