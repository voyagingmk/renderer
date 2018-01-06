#include "stdafx.h"
#include "system/physicsSys.hpp"


namespace renderer {
    void PhysicsSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("PhysicsSystem init\n");
        
    }
    
    void PhysicsSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        auto com = m_objMgr->getSingletonComponent<PhysicsWorld>();
        com->accumulator += dt;
        while (com->accumulator > com->timestep) {
            com->accumulator -= com->timestep;
            if (com->world.getNbRigidBodies() > 0) {
                com->world.update(com->timestep);
            }
        }
    }
    
    
    rp3d::CollisionBody* CreateCollisionBody(rp3d::DynamicsWorld* world, rp3d::Transform* transform)  {
        return world->createCollisionBody(*transform);
        
    }
}
