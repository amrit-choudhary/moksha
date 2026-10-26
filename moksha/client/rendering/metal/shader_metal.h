#pragma once
#ifdef ME_METAL

/**
 * Shader class for Metal, the counterpart of the DX12 Shader: loads the .metallib the mac-metal cook compiled from
 * shaders/metal/<name>.metal (Resources, through Vfs). Entry points are VS and PS, as in HLSL.
 */

namespace MTL {
class Library;
}  // namespace MTL

namespace MTL4 {
class Compiler;
}  // namespace MTL4

namespace ME {

class ShaderMetal {
   public:
    // Loads the library compiled from shaders/metal/<shaderName> (a .metal name) for compiler's device.
    // GetLibrary() is null if it is missing or invalid (logged).
    ShaderMetal(MTL4::Compiler* compiler, const char* shaderName);
    ~ShaderMetal();

    ShaderMetal(const ShaderMetal&) = delete;
    ShaderMetal& operator=(const ShaderMetal&) = delete;

    MTL::Library* GetLibrary() const;

   private:
    MTL::Library* library = nullptr;
};

}  // namespace ME

#endif  // ME_METAL
