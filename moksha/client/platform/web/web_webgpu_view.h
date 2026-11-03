#pragma once
#if defined(ME_WEB) && defined(ME_WEBGPU)

/**
 * WebGPUView over the page's canvas. The driver fills it in once the browser has given it a device.
 */

#include "moksha/client/rendering/webgpu/webgpu_view.h"

namespace ME {

class WebWebGPUView : public WebGPUView {
   public:
    WGPUAdapter GetAdapter() const override {
        return adapter;
    }
    WGPUDevice GetDevice() const override {
        return device;
    }
    WGPUSurface GetSurface() const override {
        return surface;
    }
    uint32_t GetPixelWidth() const override {
        return pixelWidth;
    }
    uint32_t GetPixelHeight() const override {
        return pixelHeight;
    }

    WGPUAdapter adapter = nullptr;
    WGPUDevice device = nullptr;
    WGPUSurface surface = nullptr;
    uint32_t pixelWidth = 0;
    uint32_t pixelHeight = 0;
};

}  // namespace ME

#endif  // ME_WEB && ME_WEBGPU
