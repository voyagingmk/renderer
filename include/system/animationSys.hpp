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
        
        bool DrawPosture(const ozz::animation::Skeleton& skeleton,
                         ozz::Range<const ozz::math::Float4x4> matrices,
                         const ozz::math::Float4x4& transform,
                         bool draw_joints = true);
        
        int DrawPosture_FillUniforms(const ozz::animation::Skeleton& skeleton,
                                                      ozz::Range<const ozz::math::Float4x4> matrices,
                                                      float* uniforms, int max_instances);
        
        void DrawPosture_InstancedImpl(const ozz::math::Float4x4& transform, const float* uniforms,
                                                        int instance_count, bool draw_joints);
    };
    
};

#endif

