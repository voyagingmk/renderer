#ifndef RENDERER_RENDERQUEUE_HPP
#define RENDERER_RENDERQUEUE_HPP

#include "base.hpp"
#include "realtime/model.hpp"
#include "material.hpp"

namespace renderer {
    
    typedef size_t RenderQueueID;
    
    class RenderItem;
    
    class RenderQueue {
        std::map<RenderQueueID, RenderItem> items;
    };
    
    class RenderQueueSet {
        RenderQueue opaqueQueue;
        RenderQueue alphaQueue;
    };
    
    class RenderItem {
    public:
        Model *model; // Not sure
        Material* mat;
        MaterialSetting* matSetting;
       float depth; // Distance from model to the camera
    };
    
}
#endif // RENDERER_RENDERQUEUE_HPP
