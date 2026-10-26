#pragma once
#ifdef ME_METAL

/**
 * What the Metal renderer needs from the window's view, as metal-cpp objects. The platform driver implements it over
 * its native view (platform/mac: an MTKView), so the renderer stays plain C++. Frame objects are valid only while
 * the frame is drawn, from inside the view's draw.
 */

namespace CA {
class MetalDrawable;
}  // namespace CA

namespace MTL {
class Device;
class RenderPassDescriptor;
}  // namespace MTL

namespace ME {

class MetalView {
   public:
    virtual ~MetalView() = default;

    // The GPU the view presents with. Not owned; retain it to keep it.
    virtual MTL::Device* GetDevice() const = 0;

    // A render pass into this frame's drawable, or nullptr when the view has none to give (e.g. the window is
    // hidden). Obtains the drawable, so call it as late as possible.
    virtual MTL::RenderPassDescriptor* GetCurrentRenderPass() const = 0;

    // This frame's drawable, or nullptr together with GetCurrentRenderPass.
    virtual CA::MetalDrawable* GetCurrentDrawable() const = 0;
};

}  // namespace ME

#endif  // ME_METAL
