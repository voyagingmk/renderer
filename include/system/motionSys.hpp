#ifndef RENDERER_SYSTEM_MOTION_HPP
#define RENDERER_SYSTEM_MOTION_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/motionCom.hpp"
#include "com/actionCom.hpp"
#include "event/actionEvent.hpp"


using namespace ecs;

namespace renderer {
	class MotionSystem : public System<MotionSystem>, public Receiver<MotionSystem>
	{
	public:
		void init(ObjectManager &objMgr, EventManager &evtMgr) override;
		
        virtual void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
        
        void receive(const AddActionEvent& event);
        
    private:
        void BeginAction(Object obj, MoveByAction* ac);
        
        void InterpolateAction(Object obj, MoveByAction* ac, float p, float duration, float dt);
        
        void BeginAction(Object obj, RotateByAction* ac);
        
        void InterpolateAction(Object obj, RotateByAction* ac, float p, float duration, float dt);
        
    };
};


#endif
