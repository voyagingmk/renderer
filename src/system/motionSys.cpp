#include "stdafx.h"
#include "system/motionSys.hpp"
#include "com/motionCom.hpp"
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
                switch(base->type) {
                    case ActionType::MoveBy: {
                        MoveByAction* ac = dynamic_cast<MoveByAction*>(base);
                        if (acData.state == 0) {
                            ac->o = spatial->pos;
                            acData.state = 1;
                        } else {
                            acData.t += dt;
                            float p = acData.t / ac->duration;
                            // interpolate
                            Vector3dF pos = ac->o + ac->by * p;
                            spatial->pos = pos;
                            m_evtMgr->emit<UpdateSpatialDataEvent>(obj);
                            if(p >= 1.0f) {
                                acData.t = 0;
                                acData.state = 0;
                                acData.idx++;
                                pos.debug();
                            }
                        }
                        
                        break;
                    }
                    default: {
                        break;
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


};
