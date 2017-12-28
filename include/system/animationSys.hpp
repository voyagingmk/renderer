#ifndef RENDERER_SYSTEM_ANIMATION_HPP
#define RENDERER_SYSTEM_ANIMATION_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/animation.hpp"


using namespace ecs;

namespace renderer {
    class AnimationSystem : public System<AnimationSystem>, public Receiver<AnimationSystem>
    {
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;
        
        void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
        
    private:
        
        void UpdateAnimationTime(AnimationCom com, float dt);
        
        bool LoadSkeleton(const char* _filename, ozz::animation::Skeleton* _skeleton);
        
        bool LoadAnimation(const char* _filename, ozz::animation::Animation* _animation);
    };
    
};

#endif

