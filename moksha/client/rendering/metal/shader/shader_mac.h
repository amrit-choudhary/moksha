#ifdef ME_MAC
/**
 * Shader class
 * This will load a shader from a file and compile it.
 * It will also provide access to the vertex and fragment functions.
 */

#pragma once

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

namespace ME {

class Shader {
   public:
    Shader(MTL::Device* device, const char* shaderName);
    ~Shader();

    MTL::Function* GetVertexFunction() const;
    MTL::Function* GetFragmentFunction() const;

   private:
    MTL::Device* device;
    MTL::Library* library;
    MTL::Function* vertexFunction;
    MTL::Function* fragmentFunction;
};

}  // namespace ME

#endif  // ME_MAC