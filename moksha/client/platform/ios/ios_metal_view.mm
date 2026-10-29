#if defined(ME_IOS) && defined(ME_METAL)

#include "ios_metal_view.h"

namespace {

// Under ARC a returned frame object (the render pass) is released when the call ends, though nothing else may hold
// it. Parking it in the autorelease pool keeps it alive for the frame, as on the non-ARC macOS adapter.
void* KeepForFrame(id object) {
    return object != nil ? const_cast<void*>(CFAutorelease(CFBridgingRetain(object))) : nullptr;
}

}  // namespace

MTL::Device* ME::IosMetalView::GetDevice() const {
    return reinterpret_cast<MTL::Device*>((__bridge void*)view.device);
}

CA::MetalLayer* ME::IosMetalView::GetLayer() const {
    return reinterpret_cast<CA::MetalLayer*>((__bridge void*)view.layer);
}

void ME::IosMetalView::SetVsync(bool enabled) const {
    // iOS always presents on the display's refresh; the frame rate is the view's preferredFramesPerSecond.
    (void)enabled;
}

MTL4::RenderPassDescriptor* ME::IosMetalView::GetCurrentRenderPass() const {
    return reinterpret_cast<MTL4::RenderPassDescriptor*>(KeepForFrame(view.currentMTL4RenderPassDescriptor));
}

CA::MetalDrawable* ME::IosMetalView::GetCurrentDrawable() const {
    return reinterpret_cast<CA::MetalDrawable*>(KeepForFrame(view.currentDrawable));
}

#endif  // ME_IOS && ME_METAL
