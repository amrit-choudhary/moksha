#ifdef ME_METAL

/**
 * Metal's specific mesh class.
 * Has MTL buffers for vertex, index and instance data.
 * This class is used to load and manage meshes using Metal.
 */

#pragma once

#include <simd/simd.h>

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

#include "moksha/client/rendering/shared/mesh.h"
#include "moksha/client/rendering/shared/texture.h"

namespace ME {

/**
 * Mesh class for handling meshes in Metal.
 * This class is used to load and manage meshes using Metal.
 */
class MeshMetal : public Mesh {
   public:
    /**
     * 1. Loads a OBJ mesh from file.
     * 2. Create Metal buffers for vertex, index and instance data.
     * 3. Upload the data to the GPU.
     */
    MeshMetal(const char* path, MTL::Device* device, MTL::CommandQueue* cmdQueue);
    ~MeshMetal() override;

    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
};

}  // namespace ME

#endif
