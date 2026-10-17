#ifdef ME_METAL
/**
 * Metal's specific depth stencil state class.
 */

#pragma once

#include <simd/simd.h>

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

#include "moksha/client/rendering/shared/material.h"

namespace ME {

/**
 * DepthStencilStateMetal class for handling depth stencil state in Metal.
 * This class is used to create and manage the depth stencil state for Metal rendering.
 */
class DepthStencilStateMetal {
   public:
    DepthStencilStateMetal();
    ~DepthStencilStateMetal();

    static MTL::DepthStencilState* GetNewDepthStencilState(
        MTL::Device* device, ME::DepthCompareFunction depthCompareFunction = ME::DepthCompareFunction::Less,
        bool depthWriteEnabled = true);

    static MTL::DepthStencilState* GetNewDepthStencilState2D(MTL::Device* device);

   private:
};

}  // namespace ME

#endif
