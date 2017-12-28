#ifndef RENDERER_ANIMATION_HPP
#define RENDERER_ANIMATION_HPP

#include "ozz/ozz.h"

struct Animation {
    // Runtime skeleton.
    ozz::animation::Skeleton skeleton;
    
    // Runtime animation.
    ozz::animation::Animation animation;
};

struct AnimationCom {
    // Sampling cache.
    ozz::animation::SamplingCache* cache;
    // Buffer of local transforms as sampled from animation_.
    ozz::Range<ozz::math::SoaTransform> locals;
    // Buffer of model space matrices.
    ozz::Range<ozz::math::Float4x4> models;
    bool play;
    float time;
    float playback_speed;
};

#endif
