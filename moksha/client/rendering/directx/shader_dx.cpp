#ifdef ME_DX12

#include "shader_dx.h"

#include <D3Dcompiler.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <string>

#include "moksha/logging/logging.h"
#include "moksha/shared/file_io/vfs.h"

namespace {

// Reads a compiled shader from the resources into a new blob; null if it is missing or unreadable.
ID3DBlob* ReadShaderBlob(const std::string& relPath) {
    size_t size = 0;
    if (!ME::Vfs::GetFileSize(ME::FileRoot::Resources, relPath.c_str(), size) || size == 0) {
        ME::LogError("Shader not found:", relPath);
        return nullptr;
    }
    ID3DBlob* blob = nullptr;
    if (FAILED(D3DCreateBlob(size, &blob))) {
        return nullptr;
    }
    if (!ME::Vfs::ReadBytes(ME::FileRoot::Resources, relPath.c_str(), static_cast<uint8_t*>(blob->GetBufferPointer()),
                            blob->GetBufferSize(), size)) {
        ME::LogError("Shader unreadable:", relPath);
        blob->Release();
        return nullptr;
    }
    return blob;
}

}  // namespace

ME::Shader::Shader(const char* shaderName) {
    CompileShader(shaderName);
}

ME::Shader::~Shader() {
    if (vsBlob) {
        vsBlob->Release();
        vsBlob = nullptr;
    }
    if (psBlob) {
        psBlob->Release();
        psBlob = nullptr;
    }
}

void ME::Shader::CompileShader(const char* shaderName) {
    std::string shaderBaseName{shaderName};
    size_t pos = shaderBaseName.find_last_of('.');
    // Remove extension if present.
    if (pos != std::string::npos) {
        shaderBaseName = shaderBaseName.substr(0, pos);
    }

    vsBlob = ReadShaderBlob("shaders/hlsl/" + shaderBaseName + "_vs.cso");
    psBlob = ReadShaderBlob("shaders/hlsl/" + shaderBaseName + "_ps.cso");
}

ID3DBlob* ME::Shader::GetVSBlob() const {
    return vsBlob;
}
ID3DBlob* ME::Shader::GetPSBlob() const {
    return psBlob;
}

#endif  // ME_DX12
