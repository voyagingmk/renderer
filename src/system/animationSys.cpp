#include "stdafx.h"
#include "system/animationSys.hpp"

using namespace std;

Animation a;
AnimationCom com;

namespace renderer {
    void AnimationSystem::init(ObjectManager &objMgr, EventManager &evtMgr) {
        printf("AnimationSystem init\n");
        ozz::options::internal::Registrer<ozz::options::StringOption> OPTIONS_skeleton(
            "skeleton", "", "assets/animation/skeleton.ozz", false);
        // Reading skeleton.
        if (!LoadSkeleton(OPTIONS_skeleton, &a.skeleton)) {
            return;
        }
        ozz::options::internal::Registrer<ozz::options::StringOption> OPTIONS_animation(
            "animation", "", "assets/animation/animation.ozz", false);
        // Reading animation.
        if (!LoadAnimation(OPTIONS_animation, &a.animation)) {
            return;
        }
        ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
        // Allocates runtime buffers.
        const int num_soa_joints = a.skeleton.num_soa_joints();
        com.locals = allocator->AllocateRange<ozz::math::SoaTransform>(num_soa_joints);
        const int num_joints = a.skeleton.num_joints();
        com.models = allocator->AllocateRange<ozz::math::Float4x4>(num_joints);
        // Allocates a cache that matches animation requirements.
        com.cache = allocator->New<ozz::animation::SamplingCache>(num_joints);
    }
    
    void AnimationSystem::update(ObjectManager &objMgr, EventManager &evtMgr, float dt) {
        // Updates current animation time.
        UpdateAnimationTime(com, dt);
        
        // Samples optimized animation at t = animation_time_.
        ozz::animation::SamplingJob sampling_job;
        sampling_job.animation = &a.animation;
        sampling_job.cache = com.cache;
        sampling_job.time = com.time;
        sampling_job.output = com.locals;
        if (!sampling_job.Run()) {
            return;
        }
        
        // Converts from local space to model space matrices.
        ozz::animation::LocalToModelJob ltm_job;
        ltm_job.skeleton = &a.skeleton;
        ltm_job.input = com.locals;
        ltm_job.output = com.models;
        if (!ltm_job.Run()) {
            return;
        }
    }
    
    bool AnimationSystem::LoadSkeleton(const char* _filename, ozz::animation::Skeleton* _skeleton) {
        assert(_filename && _skeleton);
        ozz::log::Out() << "Loading skeleton archive " << _filename << "."
        << std::endl;
        ozz::io::File file(_filename, "rb");
        if (!file.opened()) {
            ozz::log::Err() << "Failed to open skeleton file " << _filename << "."
            << std::endl;
            return false;
        }
        ozz::io::IArchive archive(&file);
        if (!archive.TestTag<ozz::animation::Skeleton>()) {
            ozz::log::Err() << "Failed to load skeleton instance from file "
            << _filename << "." << std::endl;
            return false;
        }
        
        // Once the tag is validated, reading cannot fail.
        archive >> *_skeleton;
        
        return true;
    }
    
    bool AnimationSystem::LoadAnimation(const char* _filename,
                       ozz::animation::Animation* _animation) {
        assert(_filename && _animation);
        ozz::log::Out() << "Loading animation archive: " << _filename << "."
        << std::endl;
        ozz::io::File file(_filename, "rb");
        if (!file.opened()) {
            ozz::log::Err() << "Failed to open animation file " << _filename << "."
            << std::endl;
            return false;
        }
        ozz::io::IArchive archive(&file);
        if (!archive.TestTag<ozz::animation::Animation>()) {
            ozz::log::Err() << "Failed to load animation instance from file "
            << _filename << "." << std::endl;
            return false;
        }
        
        // Once the tag is validated, reading cannot fail.
        archive >> *_animation;
        
        return true;
    }
    
    void AnimationSystem::UpdateAnimationTime(AnimationCom com, float dt) {
        const float new_time = com.time + dt * com.playback_speed;
        const float loops = new_time / a.animation.duration();
        com.time = new_time - floorf(loops) * a.animation.duration();
    }
    
};
