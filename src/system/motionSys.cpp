#include "stdafx.h"
#include "system/motionSys.hpp"
#include "com/spatialData.hpp"
#include "event/spatialEvent.hpp"

using namespace std;

namespace renderer {


	void MotionSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        evtMgr.on<AddActionEvent>(*this);
    }
    

	void MotionSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        for (const Object obj : m_objMgr->entities<SpatialData, MotionCom>()) {
            auto spatial = obj.component<SpatialData>();
            auto com = obj.component<MotionCom>();
            for(auto it = com->acContainer.begin(); it != com->acContainer.end(); it++) {
                ActionData& acData = it->second;
                // printf("acData t: %.2f, idx: %u, state: %u  \n", acData.t, acData.idx, acData.state);
                auto ptr = acData.actions[acData.idx];
                ActionBase* base = ptr.get();
                if (acData.state == 0) {
                    acData.state = 1;
                    switch(base->type) {
                        case ActionType::MoveBy: {
                            BeginAction(obj, dynamic_cast<MoveByAction*>(base));
                            break;
                        }
                        case ActionType::RotateBy: {
                            BeginAction(obj, dynamic_cast<RotateByAction*>(base));
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                } else {
                    acData.t += dt;
                    float p = acData.t / base->duration;
                    switch(base->type) {
                        case ActionType::MoveBy: {
                            InterpolateAction(obj, dynamic_cast<MoveByAction*>(base), p);
                            break;
                        }
                        case ActionType::RotateBy: {
                            InterpolateAction(obj, dynamic_cast<RotateByAction*>(base), p);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    if(p >= 1.0f) {
                        acData.t = 0;
                        acData.state = 0;
                        acData.idx++;
                    }
                }
                if (acData.idx == acData.actions.size()) {
                    if (acData.repeat == -1) {
                        acData.idx = 0;
                    }
                }
                
            }
        }
	}
    
    void MotionSystem::receive(const AddActionEvent& evt) {
        auto com = evt.obj.component<MotionCom>();
        if(!com.valid()) {
            return;
        }
        com->acContainer.insert(std::make_pair(evt.name, evt.actionData));
    }

    void MotionSystem::BeginAction(Object obj, MoveByAction* ac) {
        auto spatial = obj.component<SpatialData>();
        ac->o = spatial->pos;
    }
    
    void MotionSystem::InterpolateAction(Object obj, MoveByAction* ac, float p) {
        auto spatial = obj.component<SpatialData>();
        Vector3dF pos = ac->o + ac->by * p;
        spatial->pos = pos;
        m_evtMgr->emit<UpdateSpatialDataEvent>(obj);
    }
    
    void MotionSystem::BeginAction(Object obj, RotateByAction* ac) {
        auto spatial = obj.component<SpatialData>();
        ac->o = spatial->orientation;
    }
    
    void MotionSystem::InterpolateAction(Object obj, RotateByAction* ac, float p) {
        auto spatial = obj.component<SpatialData>();
        spatial->orientation = ac->o + ac->diff * p;
        // spatial->orientation.debug();
        m_evtMgr->emit<UpdateSpatialDataEvent>(obj);
    }
};
