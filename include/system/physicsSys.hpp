#ifndef RENDERER_SYSTEM_PHYSICS_HPP
#define RENDERER_SYSTEM_PHYSICS_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"

using namespace ecs;

namespace renderer {
    class PhysicsSystem : public System<PhysicsSystem>, public Receiver<PhysicsSystem>
    {
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;
        
        void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
        
    private:
        
    };
    
};


#endif
