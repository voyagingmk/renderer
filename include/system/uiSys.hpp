#ifndef RENDERER_SYSTEM_UI_HPP
#define RENDERER_SYSTEM_UI_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "event/uiEvent.hpp"
#include "event/winEvent.hpp"


using namespace ecs;

namespace renderer {
    class UISystem : public System<UISystem>, public Receiver<UISystem>
	{
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;
   
        void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
        
        void receive(const DrawUIEvent &evt);
     
        void receive(const SetupSDLDoneEvent &evt);
	};

};


#endif
