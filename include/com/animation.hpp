#ifndef RENDERER_ANIMATION_HPP
#define RENDERER_ANIMATION_HPP

#include "ozz/ozz.h"
#include <map>

typedef size_t AnimationID;

class AnimationData {
 public:
    AnimationData() {}
    // Runtime skeleton.
    ozz::animation::Skeleton skeleton;
    // Runtime animation.
    std::map<std::string, ozz::animation::Animation> aniDict;
    ozz::animation::Animation* GetAnimation(std::string aniName) {
        return &aniDict[aniName];
    }
 private:
    // Disables copy and assignation.
    AnimationData(AnimationData const&);
    void operator=(AnimationData const&);

};

class AnimationDataSet {
public:
    AnimationID newAnimationID() {
        return ++idCount;
    }
    
    AnimationID getAnimationID(std::string aliasName) {
        return alias2id[aliasName];
    }
    
    
    bool hasAnimationData(std::string aliasName) {
       return alias2id.find(aliasName) != alias2id.end();
    }
    
    bool hasAnimationData(AnimationID id) {
        return animations.find(id) != animations.end();
    }
    
    AnimationData& getAnimationData(AnimationID id) {
        return animations[id];
    }
    
    AnimationData& getAnimationData(std::string aliasName) {
        return animations[alias2id[aliasName]];
    }
    AnimationID idCount;
    std::map<std::string, AnimationID> alias2id;
    std::map<AnimationID, AnimationData> animations;
};

class AnimationCom {
public:
    AnimationCom(AnimationID id, AnimationData& data) {
        animationID = id;
        ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
        // Allocates runtime buffers.
        const int num_soa_joints = data.skeleton.num_soa_joints();
        locals = allocator->AllocateRange<ozz::math::SoaTransform>(num_soa_joints);
        const int num_joints = data.skeleton.num_joints();
        models = allocator->AllocateRange<ozz::math::Float4x4>(num_joints);
        // Allocates a cache that matches animation requirements.
        cache = allocator->New<ozz::animation::SamplingCache>(num_joints);
        skinning_matrices = allocator->AllocateRange<ozz::math::Float4x4>(num_joints);
    }
    ~AnimationCom() {
        ozz::memory::Allocator* allocator = ozz::memory::default_allocator();
        allocator->Deallocate(locals);
        allocator->Deallocate(models);
        allocator->Delete(cache);
        allocator->Deallocate(skinning_matrices);
    }
    // Sampling cache.
    ozz::animation::SamplingCache* cache;
    // Buffer of local transforms as sampled from animation_.
    ozz::Range<ozz::math::SoaTransform> locals;
    // Buffer of model space matrices.
    ozz::Range<ozz::math::Float4x4> models;
    ozz::Range<ozz::math::Float4x4> skinning_matrices;
    bool play = true;
    float time = 0.0f;
    float playback_speed = 1.0f;
    AnimationID animationID = 0;
    std::string curAniName = "";
};

#endif
