#ifndef RENDERER_COM_ACTION_HPP
#define RENDERER_COM_ACTION_HPP

#include "base.hpp"
#include "motionCom.hpp"
#include "geometry.hpp"
#include "quaternion.hpp"

namespace renderer {

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

    
    class ScaleByAction: public ActionBase {
    public:
        ScaleByAction(float duration, Vector3dF by):
            ActionBase(duration, ActionType::ScaleBy),
            by(by)
        {}
        Vector3dF by;
    };
 
    
    class RotateByAction: public ActionBase {
    public:
        RotateByAction(float duration, Axis axis, float angle):
            ActionBase(duration, ActionType::RotateBy)
        {
            diff.FromAxis(angle, axis);
        }
        
        RotateByAction(float duration, QuaternionF diff):
        ActionBase(duration, ActionType::RotateBy),
        diff(diff)
        {}
        
        QuaternionF o;
        QuaternionF diff;
    };
    
    
    class DelayTimeAction: public ActionBase {
    public:
        DelayTimeAction(float duration):
        ActionBase(duration, ActionType::DelayTime)
        {}
    };
    

};

#endif
