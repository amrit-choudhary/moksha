#pragma once
#if defined(ME_IOS) && defined(ME_METAL) && defined(__OBJC__)

/**
 * The Metal renderer's access to the iOS game view (rendering/metal/metal_view.h), an MTKView. The Objective-C
 * objects are handed over as their metal-cpp types, which wrap the same objects; ownership stays with the view.
 * Objective-C++ only.
 */

#import <MetalKit/MetalKit.h>

#include "moksha/client/rendering/metal/metal_view.h"

namespace ME {

class IosMetalView : public MetalView {
   public:
    explicit IosMetalView(MTKView* inView) : view(inView) {}

    MTL::Device* GetDevice() const override;
    CA::MetalLayer* GetLayer() const override;
    void SetVsync(bool enabled) const override;
    MTL4::RenderPassDescriptor* GetCurrentRenderPass() const override;
    CA::MetalDrawable* GetCurrentDrawable() const override;

   private:
    MTKView* view = nil;
};

}  // namespace ME

#endif  // ME_IOS && ME_METAL && __OBJC__
