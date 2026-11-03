#pragma once
#ifdef ME_WEBGPU

#include <webgpu/webgpu.h>

#include "moksha/client/rendering/shared/quad.h"

namespace ME {

class QuadWebGPU : public Quad {
   public:
    QuadWebGPU() = default;
    ~QuadWebGPU() override;

    QuadWebGPU(const QuadWebGPU&) = delete;
    QuadWebGPU& operator=(const QuadWebGPU&) = delete;

    // Create the vertex and index buffers on the GPU.
    void CreateBuffers(WGPUDevice device, WGPUQueue queue);

    WGPUBuffer vertexBuffer = nullptr;
    WGPUBuffer indexBuffer = nullptr;
};

}  // namespace ME

#endif  // ME_WEBGPU
