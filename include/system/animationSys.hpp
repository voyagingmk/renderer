#ifndef RENDERER_SYSTEM_ANIMATION_HPP
#define RENDERER_SYSTEM_ANIMATION_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/animation.hpp"
#include "event/miscEvent.hpp"


using namespace ecs;

namespace renderer {
    class AnimationSystem : public System<AnimationSystem>, public Receiver<AnimationSystem>
    {
    public:
        void init(ObjectManager &objMgr, EventManager &evtMgr) override;
        
        void update(ObjectManager &objMgr, EventManager &evtMgr, float dt) override;
        
        void receive(const DebugDrawSkeletonEvent &evt);

        void receive(const LoadAnimationEvent &evt);
        
        void receive(const GlobalComAddedEvent &evt);
        
    private:
        
        void UpdateAnimationTime(AnimationData& data, ComponentHandle<AnimationCom> com, float dt);
        
        bool InitPostureRendering();

        bool DrawPosture(const ozz::animation::Skeleton& skeleton,
                         ozz::Range<const ozz::math::Float4x4> matrices,
                         const ozz::math::Float4x4& transform,
                         bool draw_joints = true);
        
        int DrawPosture_FillUniforms(const ozz::animation::Skeleton& skeleton,
                                                      ozz::Range<const ozz::math::Float4x4> matrices,
                                                      float* uniforms, int max_instances);
        
        void DrawPosture_InstancedImpl(const ozz::math::Float4x4& transform, const float* uniforms,
                                                        int instance_count, bool draw_joints);
        
        bool LoadByConfig(const std::string& assetsDir,const json &config);
        
        bool LoadSkeleton(const std::string& assetsDir, std::string& aniDataName, std::string& skeletonFileName);
        
        bool LoadAnimation(const std::string& assetsDir, std::string& aniDataName, std::string& aniAliasName, std::string& aniFileName);
        
        bool LoadSkeleton(const char* _filename, ozz::animation::Skeleton* _skeleton);
        
        bool LoadAnimation(const char* _filename, ozz::animation::Animation* _animation);
        
        void DoSamplingJob(float time, ozz::animation::SamplingCache* cache, ozz::animation::Animation* animation,  ozz::Range<ozz::math::SoaTransform> locals);
        
        void DoLocalToModelJob(ozz::animation::Skeleton& skeleton,
                               ozz::Range<ozz::math::SoaTransform> locals,
                               ozz::Range<ozz::math::Float4x4> models);
        
    };
    
};

#endif

