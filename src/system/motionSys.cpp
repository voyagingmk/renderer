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
		std::vector<std::string> expired;
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
                            InterpolateAction(obj, dynamic_cast<MoveByAction*>(base), p, base->duration, dt);
                            break;
                        }
                        case ActionType::RotateBy: {
                            InterpolateAction(obj, dynamic_cast<RotateByAction*>(base), p, base->duration, dt);
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
					else if (acData.repeat > 0) {
						acData.repeat--;
						expired.push_back(it->first);
					}
                }
            }
			for (auto name: expired) {
				com->acContainer.erase(name);
			}
        }
	}
    
    void MotionSystem::receive(const AddActionEvent& evt) {
        auto com = evt.obj.component<MotionCom>();
        if(!com.valid()) {
            return;
        }
		if (com->acContainer.find(evt.name) != com->acContainer.end()) {
			return;
		}
        com->acContainer.insert(std::make_pair(evt.name, evt.actionData));
    }

    void MotionSystem::BeginAction(Object obj, MoveByAction* ac) {
        auto spatial = obj.component<SpatialData>();
        ac->o = spatial->pos;
    }
    
    void MotionSystem::InterpolateAction(Object obj, MoveByAction* ac, float p, float duration, float dt) {
        auto spatial = obj.component<SpatialData>();
        Vector3dF pos = ac->o + ac->by * p;
        spatial->pos = pos;
        m_evtMgr->emit<UpdateSpatialDataEvent>(obj);
    }
    
    void MotionSystem::BeginAction(Object obj, RotateByAction* ac) { 
		auto spatial = obj.component<SpatialData>();
		ac->o = spatial->orientation;
		ac->to = spatial->orientation * ac->by;
	}
    
    void MotionSystem::InterpolateAction(Object obj, RotateByAction* ac, float p, float duration, float dt) {
        auto spatial = obj.component<SpatialData>();
       // auto q = ac->diff * (dt / duration);
        // spatial->orientation = q * spatial->orientation * q.Inverse();
       // qt=sin((1−t)θ)sinθq1+sin(tθ)sinθq2
        // spatial->orientation = spatial->orientation.Normalize();
		// spatial->orientation = ac->o + (ac->to - ac->o) * p;
		// spatial->orientation = ac->o + (ac->o.Inverse() * ac->to) * p;
		float dot =ac->o.Dot(ac->to);
		float norm = ac->o.Norm() * ac->to.Norm();
		float theta = acos( dot / norm);
		float sintheta = sin(theta);
		if (almost_equal(sintheta, 0.f, 2)) {
			return;
		}
		QuaternionF q = (ac->o * sin((1 - p) * theta)) / sintheta + (ac->to * (p * theta)) / sintheta;
		spatial->orientation = q;

		// spatial->orientation = spatial->orientation.Normalize();
        // spatial->orientation.debug();
        m_evtMgr->emit<UpdateSpatialDataEvent>(obj);
    }
};
