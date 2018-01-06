#include "stdafx.h"
#include "system/physicsSys.hpp"
#include "com/spatialData.hpp"
#include "com/quaternion.hpp"
#include "com/miscCom.hpp"

namespace renderer {
    
    rp3d::Quaternion Trans(QuaternionF& q) {
        return rp3d::Quaternion(q.x, q.y, q.z, q.s);
    }

    
    void PhysicsSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("PhysicsSystem init\n");
        evtMgr.on<CreateCollisionShapeEvent>(*this);
        //rp3d::decimal radius = rp3d::decimal(3.0)
        //const rp3d::BoxShape shape(radius);
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
    
    void PhysicsSystem::receive(const CreateCollisionShapeEvent& evt) {
        Object obj = evt.obj;
        if (obj.hasComponent<ColBodyCom>()) {
            return;
        }
        auto spatialData = obj.component<SpatialData>();
        auto physicsWorld = m_objMgr->getSingletonComponent<PhysicsWorld>();
        rp3d::Vector3 initPosition(spatialData->pos.x, spatialData->pos.y, spatialData->pos.z);
        rp3d::Quaternion initOrientation = Trans(spatialData->orientation);
        const rp3d::Transform transform(initPosition, initOrientation);
        rp3d::CollisionBody* body = physicsWorld->world.createCollisionBody(transform);
        auto com = obj.addComponent<ColBodyCom>();
        com->body = body;
        if (obj.hasComponent<StaticObjTag>()) {
            com->body->setType(rp3d::BodyType::STATIC);
        } else if (obj.hasComponent<DynamicObjTag>()) {
            com->body->setType(rp3d::BodyType::DYNAMIC);
        }
    }

}
