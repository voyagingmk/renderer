#ifndef RENDERER_COM_MOTION_HPP
#define RENDERER_COM_MOTION_HPP

#include "base.hpp"
#include "geometry.hpp"

namespace renderer {
    
    enum class ActionType {
        MoveBy,
        MoveTo,
        ScaleBy,
        ScaleTo,
        DelayTime
    };

    class ActionBase {
    public:
        ActionBase(float duration, ActionType type):
            duration(duration),
            type(type)
        {}
        virtual ~ActionBase() {}
        ActionType type;
        float duration;
    };

    class MoveByAction: public ActionBase {
    public:
        MoveByAction(float duration, Vector3dF by):
            ActionBase(duration, ActionType::MoveBy),
            by(by)
        {}
        Vector3dF o;
        Vector3dF by;
    };

    class MoveToAction: public ActionBase {
    public:
        MoveToAction(float duration, Vector3dF to):
            ActionBase(duration, ActionType::MoveTo),
            to(to)
        {}
        Vector3dF to;
    };

    typedef std::vector<std::shared_ptr<ActionBase>> Actions;
    
    class ActionData {
    public:
        ActionData():
            idx(0),
            t(0),
            state(0),
            repeat(0)
        {
            // printf("ActionData ctor\n");
        }
        ~ActionData() {
            // printf("ActionData dtor\n");
            // printf("acData t: %.2f, idx: %u, state: %u  \n", t, idx, state);
        }
        
        Actions actions;
        uint32_t idx; // current action
        float t;
        uint8_t state; // 0: not started  1: started
        size_t repeat; // 0: no repeat   -1: forever     >0: repeat n times
    };
    
    typedef std::map<std::string, ActionData> ActionContainer;
    
	struct MotionCom {
    public:
        ActionContainer acContainer;
    };
}

#endif
