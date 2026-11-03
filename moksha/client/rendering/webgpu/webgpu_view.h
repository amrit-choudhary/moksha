#pragma once
#ifdef ME_WEBGPU

/**
 * What the WebGPU renderer needs from the platform's window. The platform creates these objects before the app starts,
 * since a device arrives asynchronously, and implements this over its own window (platform/web: a canvas). Not owned.
 */

#include <webgpu/webgpu.h>

#include <cstdint>

namespace ME {

class WebGPUView {
   public:
    virtual ~WebGPUView() = default;

    virtual WGPUAdapter GetAdapter() const = 0;
    virtual WGPUDevice GetDevice() const = 0;
    virtual WGPUSurface GetSurface() const = 0;

    // The surface's size in pixels (window size times dpi scale).
    virtual uint32_t GetPixelWidth() const = 0;
    virtual uint32_t GetPixelHeight() const = 0;
};

}  // namespace ME

#endif  // ME_WEBGPU
