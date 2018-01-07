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
        RotateToAction(float duration, DegreeF x, DegreeF y, DegreeF z):
        ActionBase(duration, ActionType::RotateTo)
        {
            to.FromEulerAngles(x, y, x);
        }
        
        
        RotateToAction(float duration, RadianF x, RadianF y, RadianF z):
            ActionBase(duration, ActionType::RotateTo)
        {
            to.FromEulerAngles(x, y, z);
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
			x: x counter-clockwise
			y: y counter-clockwise
			z: z counter-clockwise
		*/
        RotateByAction(float duration, DegreeF x, DegreeF y, DegreeF z, bool inverse = false):
			ActionBase(duration, ActionType::RotateBy),
			inverse(inverse)
        {
            by.FromEulerAngles(x, y, z);
        }
        
        RotateByAction(float duration, RadianF x, RadianF y, RadianF z, bool inverse = false):
			ActionBase(duration, ActionType::RotateBy),
			inverse(inverse)
        {
            by.FromEulerAngles(x, y, z);
        }
        
        RotateByAction(float duration, QuaternionF by, bool inverse = false):
			ActionBase(duration, ActionType::RotateBy),
			by(by),
			inverse(inverse)
        {}
        QuaternionF o;
        QuaternionF by;
		QuaternionF to;
		bool inverse;
    };
    
    class DelayTimeAction: public ActionBase {
    public:
        DelayTimeAction(float duration):
        ActionBase(duration, ActionType::DelayTime)
        {}
    };
    

};

#endif
