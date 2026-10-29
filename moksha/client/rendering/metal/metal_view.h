#pragma once
#ifdef ME_METAL

/**
 * What the Metal renderer needs from the window's view, as metal-cpp objects. The platform implements it over its
 * native view (platform/mac: an MTKView), so the renderer stays plain C++ and OS-neutral. Frame objects are valid
 * only while the frame is drawn, from inside the view's draw.
 */

namespace CA {
class MetalDrawable;
class MetalLayer;
}  // namespace CA

namespace MTL {
class Device;
}  // namespace MTL

namespace MTL4 {
class RenderPassDescriptor;
}  // namespace MTL4

namespace ME {

class MetalView {
   public:
    virtual ~MetalView() = default;

    // The GPU the view presents with. Not owned; retain it to keep it.
    virtual MTL::Device* GetDevice() const = 0;

    // The layer the view presents to (drawable size, its residency set). Not owned.
    virtual CA::MetalLayer* GetLayer() const = 0;

    // Asks for presents synced to the display, or as fast as possible. An OS that always syncs (iOS) ignores it.
    virtual void SetVsync(bool enabled) const = 0;

    // A Metal 4 render pass into this frame's drawable, or nullptr when the view has none to give (e.g. the window
    // is hidden). Obtains the drawable, so call it as late as possible.
    virtual MTL4::RenderPassDescriptor* GetCurrentRenderPass() const = 0;

    // This frame's drawable, or nullptr together with GetCurrentRenderPass.
    virtual CA::MetalDrawable* GetCurrentDrawable() const = 0;
};

}  // namespace ME

#endif  // ME_METAL
