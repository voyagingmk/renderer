#ifndef RENDERER_COM_PHYSICS_HPP
#define RENDERER_COM_PHYSICS_HPP

#include "reactphysics3d/reactphysics3d.h"

namespace renderer {

class PhysicsWorld {
public:
    PhysicsWorld():
        world(rp3d::Vector3(0.0, -9.81, 0.0)),
        accumulator(0.0f),
        timestep(1.0f / 60.0f)
    {}
    // rp3d::CollisionWorld world;
    rp3d::DynamicsWorld world;
    float accumulator;
    const float timestep;
};
    
typedef size_t CollisionShapeID;
    
class CollisionShapeSet {
public:
    CollisionShapeSet() :
        idCount(0) {}
    CollisionShapeID newID() { return ++idCount; }
    rp3d::CollisionShape* getShape(CollisionShapeID id) {
        return shapeDict[id];
    }
    CollisionShapeID idCount;
    std::map<CollisionShapeID, rp3d::CollisionShape*> shapeDict;
    std::map<std::string, CollisionShapeID> alias2id;
};
    
class ColBodyCom {
public:
    rp3d::RigidBody* body;
    rp3d::Transform prevTransform;
};

};

#endif
