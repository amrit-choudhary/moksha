#ifdef ME_DX12

#include "shader_dx.h"

#include <D3Dcompiler.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <string>

#include "moksha/shared/misc/utils.h"

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

    std::string filePathVS = ME::Utils::GetResourcesPath() + "shaders/hlsl/" + shaderBaseName + "_vs.cso";
    std::string filePathPS = ME::Utils::GetResourcesPath() + "shaders/hlsl/" + shaderBaseName + "_ps.cso";

    D3DReadFileToBlob(std::wstring(filePathVS.begin(), filePathVS.end()).c_str(), &vsBlob);
    D3DReadFileToBlob(std::wstring(filePathPS.begin(), filePathPS.end()).c_str(), &psBlob);
}

ID3DBlob* ME::Shader::GetVSBlob() const {
    return vsBlob;
}
ID3DBlob* ME::Shader::GetPSBlob() const {
    return psBlob;
}

#endif  // ME_DX12
