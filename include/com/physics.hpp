#ifndef RENDERER_COM_PHYSICS_HPP
#define RENDERER_COM_PHYSICS_HPP

#include "reactphysics3d/reactphysics3d.h"

using namespace reactphysics3d;

namespace renderer {

class PhysicsWorld {
public:
    PhysicsWorld():
        accumulator(0.0f),
        timestep(1.0f / 60.0f),
        world(rp3d::Vector3(0.0, -9.81, 0.0))
    {}
    // rp3d::CollisionWorld world;
    rp3d::DynamicsWorld world;
    float accumulator;
    const float timestep;
};
    
class CollisionBody {
public:
    /*
    // Initial position and orientation of the collision body
    rp3d::Vector3 initPosition(0.0, 3.0, 0.0);
    rp3d::Quaternion initOrientation = rp3d::Quaternion::identity();
    rp3d::Transform transform(initPosition, initOrientation);
    */
    rp3d::CollisionBody* body;
};

};

#endif
