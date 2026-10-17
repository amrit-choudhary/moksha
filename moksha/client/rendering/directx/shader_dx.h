#pragma once
#ifdef ME_WIN

/**
 * Shader class for DirectX
 * This will load a hlsl shader from a file and compile it.
 * It will also provide access to the vertex and pixel functions.
 */

#include <d3d12.h>

#include "moksha/client/rendering/shared/renderer.h"

namespace ME {

class Shader {
   public:
    Shader(const char* shaderName);
    ~Shader();

    void Bind() const;
    void Unbind() const;
    ID3DBlob* GetVSBlob() const;
    ID3DBlob* GetPSBlob() const;

   private:
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;

    void CompileShader(const char* shaderName);
};

}  // namespace ME

#endif  // ME_WIN
