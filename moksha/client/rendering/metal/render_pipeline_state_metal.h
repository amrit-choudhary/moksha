#ifdef ME_MAC

/**
 * Metal's specific render pipeline state class.
 */

#pragma once

#include <simd/simd.h>

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

namespace ME {

/**
 * RenderPipelineStateMetal class for handling render pipeline state in Metal.
 * This class is used to create and manage the render pipeline state for Metal rendering.
 */
class RenderPipelineStateMetal {
   public:
    RenderPipelineStateMetal();
    ~RenderPipelineStateMetal();

    /** 3D PSO */
    static MTL::RenderPipelineState* GetNewPSO(MTL::Device* device,
                                               const char* shaderPath = "shaders/metal/basic.metal");

    /** 2D PSO */
    static MTL::RenderPipelineState* GetNewPSO2D(MTL::Device* device,
                                                 const char* shaderPath = "shaders/metal/sprite.metal");

   private:
};

}  // namespace ME

#endif
