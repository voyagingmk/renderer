#ifndef RENDERER_ANIMATION_HPP
#define RENDERER_ANIMATION_HPP

#include "ozz/ozz.h"
#include <map>

typedef size_t AnimationID;

class AnimationData {
 public:
    AnimationData(){}
    // Runtime skeleton.
    ozz::animation::Skeleton skeleton;
    // Runtime animation.
    std::map<std::string, ozz::animation::Animation> aniDict;
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
        return alias2id[id];
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

struct AnimationCom {
    // Sampling cache.
    ozz::animation::SamplingCache* cache;
    // Buffer of local transforms as sampled from animation_.
    ozz::Range<ozz::math::SoaTransform> locals;
    // Buffer of model space matrices.
    ozz::Range<ozz::math::Float4x4> models;
    bool play = true;
    float time = 0.0f;
    float playback_speed = 1.0f;
};

#endif
