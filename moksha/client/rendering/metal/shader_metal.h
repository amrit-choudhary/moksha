#pragma once
#ifdef ME_METAL

/**
 * Shader class for Metal, the counterpart of the DX12 Shader: reads a self-contained .metal source from
 * shaders/metal/ (Resources) and compiles it at startup with MTL4Compiler. Entry points are VS and PS, as in HLSL.
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
    // Compiles shaders/metal/<shaderName>; GetLibrary() is null if reading or compiling failed (logged).
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
