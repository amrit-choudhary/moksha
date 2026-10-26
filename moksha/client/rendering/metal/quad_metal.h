#pragma once
#ifdef ME_METAL

/**
 * Metal's quad class, the counterpart of QuadDX: the shared Quad's vertices and 16-bit indices in GPU buffers.
 * The owner adds both buffers to a residency set.
 */

#include "moksha/client/rendering/shared/quad.h"

namespace MTL {
class Buffer;
class Device;
}  // namespace MTL

namespace ME {

class QuadMetal : public Quad {
   public:
    QuadMetal();
    ~QuadMetal() override;

    QuadMetal(const QuadMetal&) = delete;
    QuadMetal& operator=(const QuadMetal&) = delete;

    // Create the vertex and index buffers on the GPU.
    void CreateBuffers(MTL::Device* device);

    MTL::Buffer* vertexBuffer = nullptr;
    MTL::Buffer* indexBuffer = nullptr;
};

}  // namespace ME

#endif  // ME_METAL
