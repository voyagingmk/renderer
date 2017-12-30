#ifndef RENDERER_ANIMATION_HPP
#define RENDERER_ANIMATION_HPP

#include "ozz/ozz.h"
#include <map>

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
    bool hasAnimationData(std::string aliasName) {
       return animations.find(aliasName) != animations.end();
    }
    
    AnimationData& getAnimationData(std::string aliasName) {
        return animations[aliasName];
    }
    
    std::map<std::string, AnimationData> animations;
    
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
