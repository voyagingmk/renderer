#include "stdafx.h"
#include "object.hpp"
#include "object_manager.hpp"

namespace ecs {

	Object::Object(const ObjectManager *manager, ObjectID id) :
		m_manager(const_cast<ObjectManager *>(manager)), m_id(id)
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

};