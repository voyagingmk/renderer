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
 
    
    class RotateToAction: public ActionBase {
    public:
        RotateToAction(float duration, DegreeF phi, DegreeF theta, DegreeF psi):
        ActionBase(duration, ActionType::RotateTo)
        {
            to.FromEulerAngles(phi, theta, psi);
        }
        
        
        RotateToAction(float duration, RadianF phi, RadianF theta, RadianF psi):
            ActionBase(duration, ActionType::RotateTo)
        {
            to.FromEulerAngles(phi, theta, psi);
        }
        
        RotateToAction(float duration, QuaternionF to):
            ActionBase(duration, ActionType::RotateTo),
            to(to)
        {}
        QuaternionF o;
        QuaternionF to;
    };
    
    
    class RotateByAction: public ActionBase {
    public:
		/* 
			alpha: x counter-clockwise
			beta:  y counter-clockwise
			gamma: z counter-clockwise
		*/
        RotateByAction(float duration, DegreeF alpha, DegreeF beta, DegreeF gamma):
        ActionBase(duration, ActionType::RotateBy)
        {
            by.FromEulerAngles(alpha, beta, gamma);
        }
        
        RotateByAction(float duration, RadianF alpha, RadianF beta, RadianF gamma):
			ActionBase(duration, ActionType::RotateBy)
        {
            by.FromEulerAngles(alpha, beta, gamma);
        }
        
        RotateByAction(float duration, QuaternionF by):
			ActionBase(duration, ActionType::RotateBy),
			by(by)
        {}
        QuaternionF o;
        QuaternionF by;
		QuaternionF to;
    };
    
    class DelayTimeAction: public ActionBase {
    public:
        DelayTimeAction(float duration):
        ActionBase(duration, ActionType::DelayTime)
        {}
    };
    

};

#endif
