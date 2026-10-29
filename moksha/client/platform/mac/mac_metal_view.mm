#if defined(ME_MAC) && defined(ME_METAL)

#include "mac_metal_view.h"

#import <QuartzCore/CAMetalLayer.h>

MTL::Device* ME::MacMetalView::GetDevice() const {
    return reinterpret_cast<MTL::Device*>((__bridge void*)view.device);
}

CA::MetalLayer* ME::MacMetalView::GetLayer() const {
    return reinterpret_cast<CA::MetalLayer*>((__bridge void*)view.layer);
}

void ME::MacMetalView::SetVsync(bool enabled) const {
    static_cast<CAMetalLayer*>(view.layer).displaySyncEnabled = enabled ? YES : NO;
}

MTL4::RenderPassDescriptor* ME::MacMetalView::GetCurrentRenderPass() const {
    return reinterpret_cast<MTL4::RenderPassDescriptor*>((__bridge void*)view.currentMTL4RenderPassDescriptor);
}

CA::MetalDrawable* ME::MacMetalView::GetCurrentDrawable() const {
    return reinterpret_cast<CA::MetalDrawable*>((__bridge void*)view.currentDrawable);
}

#endif  // ME_MAC && ME_METAL
