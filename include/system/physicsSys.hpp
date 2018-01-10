#ifndef RENDERER_SYSTEM_PHYSICS_HPP
#define RENDERER_SYSTEM_PHYSICS_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/physics.hpp"
#include "com/matrix.hpp"
#include "com/spatialData.hpp"
#include "event/physicsEvent.hpp"
#include "event/spatialEvent.hpp"


using namespace ecs;

namespace renderer {
    class PhysicsSystem : public System<PhysicsSystem>, public Receiver<PhysicsSystem>
    {
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;
        
        void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
        
        void receive(const ComponentAddedEvent<PhysicsWorld>& evt);
        
        void receive(const CreateCollisionShapeEvent& evt);

        void receive(const UpdateSpatialDataEvent &evt);
        
        void receive(const DebugDrawCollisionShapeEvent& evt);
        
    private:
        
        void ComputeTransform(ComponentHandle<ColBodyCom> com, float interpolationFactor);
        
    };
    
};


#endif
