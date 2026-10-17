#ifdef ME_DX12

#include "root_sig_dx.h"

#include "d3dx12.h"

ID3D12RootSignature* ME::RootSigDx::CreateRootSignature2D(ID3D12Device* device) {
    if (!device) return nullptr;

    CD3DX12_ROOT_PARAMETER rootParameters[3];

    // Per Pass at register(b0)
    CD3DX12_DESCRIPTOR_RANGE cbvRangePass;
    cbvRangePass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    rootParameters[0].InitAsDescriptorTable(1, &cbvRangePass);

    // Per Object at register(b1)
    CD3DX12_DESCRIPTOR_RANGE cbvRangeObject;
    cbvRangeObject.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    rootParameters[1].InitAsDescriptorTable(1, &cbvRangeObject);

    // Texture at register(t0)
    CD3DX12_DESCRIPTOR_RANGE srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[2].InitAsDescriptorTable(1, &srvRange);

    CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = GetStaticSamplerDescriptors();

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = _countof(rootParameters);
    desc.pParameters = rootParameters;
    desc.NumStaticSamplers = 6;
    desc.pStaticSamplers = staticSamplers;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* serialized;
    ID3DBlob* error;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        return nullptr;
    }

    ID3D12RootSignature* rootSig;
    hr = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
                                     IID_PPV_ARGS(&rootSig));
    if (FAILED(hr)) return nullptr;

    return rootSig;
}

ID3D12RootSignature* ME::RootSigDx::CreateRootSignature2DInstanced(ID3D12Device* device) {
    if (!device) return nullptr;

    CD3DX12_ROOT_PARAMETER rootParameters[3];

    // Per Pass at register(b0)
    CD3DX12_DESCRIPTOR_RANGE cbvRangePass;
    cbvRangePass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    rootParameters[0].InitAsDescriptorTable(1, &cbvRangePass);

    // Texture at register(t0)
    CD3DX12_DESCRIPTOR_RANGE srvRangeTex;
    srvRangeTex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[1].InitAsDescriptorTable(1, &srvRangeTex);

    // Instance Buffer at register(t1)
    CD3DX12_DESCRIPTOR_RANGE srvRangeInstance;
    srvRangeInstance.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    rootParameters[2].InitAsDescriptorTable(1, &srvRangeInstance, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = GetStaticSamplerDescriptors();

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = _countof(rootParameters);
    desc.pParameters = rootParameters;
    desc.NumStaticSamplers = 6;
    desc.pStaticSamplers = staticSamplers;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* serialized;
    ID3DBlob* error;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        return nullptr;
    }

    ID3D12RootSignature* rootSig;
    hr = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
                                     IID_PPV_ARGS(&rootSig));
    if (FAILED(hr)) return nullptr;

    return rootSig;
}

ID3D12RootSignature* ME::RootSigDx::CreateRootSignature2DInstancedAtlas(ID3D12Device* device) {
    if (!device) return nullptr;

    CD3DX12_ROOT_PARAMETER rootParameters[4];

    // Per Pass at register(b0)
    CD3DX12_DESCRIPTOR_RANGE cbvRangePass;
    cbvRangePass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    rootParameters[0].InitAsDescriptorTable(1, &cbvRangePass);

    // Texture Atlas Properties at register(b1)
    CD3DX12_DESCRIPTOR_RANGE cbvRangeAtlasProps;
    cbvRangeAtlasProps.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    rootParameters[1].InitAsDescriptorTable(1, &cbvRangeAtlasProps);

    // Texture at register(t0)
    CD3DX12_DESCRIPTOR_RANGE srvRangeTex;
    srvRangeTex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[2].InitAsDescriptorTable(1, &srvRangeTex);

    // Instance Buffer at register(t1)
    CD3DX12_DESCRIPTOR_RANGE srvRangeInstance;
    srvRangeInstance.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    rootParameters[3].InitAsDescriptorTable(1, &srvRangeInstance, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = GetStaticSamplerDescriptors();

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = _countof(rootParameters);
    desc.pParameters = rootParameters;
    desc.NumStaticSamplers = 6;
    desc.pStaticSamplers = staticSamplers;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* serialized;
    ID3DBlob* error;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        return nullptr;
    }

    ID3D12RootSignature* rootSig;
    hr = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
                                     IID_PPV_ARGS(&rootSig));
    if (FAILED(hr)) return nullptr;

    return rootSig;
}

ID3D12RootSignature* ME::RootSigDx::CreateRootSignature2DAtlas(ID3D12Device* device) {
    if (!device) return nullptr;

    CD3DX12_ROOT_PARAMETER rootParameters[4];

    // Per Pass at register(b0)
    CD3DX12_DESCRIPTOR_RANGE cbvRangePass;
    cbvRangePass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    rootParameters[0].InitAsDescriptorTable(1, &cbvRangePass);

    // Texture Atlas Properties at register(b1)
    CD3DX12_DESCRIPTOR_RANGE cbvRangeAtlasProps;
    cbvRangeAtlasProps.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    rootParameters[1].InitAsDescriptorTable(1, &cbvRangeAtlasProps);

    // Texture at register(t0)
    CD3DX12_DESCRIPTOR_RANGE srvRangeTex;
    srvRangeTex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[2].InitAsDescriptorTable(1, &srvRangeTex);

    // Per Sprite at register(b2)
    CD3DX12_DESCRIPTOR_RANGE cbvRangeSprite;
    cbvRangeSprite.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
    rootParameters[3].InitAsDescriptorTable(1, &cbvRangeSprite);

    CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = GetStaticSamplerDescriptors();

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = _countof(rootParameters);
    desc.pParameters = rootParameters;
    desc.NumStaticSamplers = 6;
    desc.pStaticSamplers = staticSamplers;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* serialized;
    ID3DBlob* error;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        return nullptr;
    }

    ID3D12RootSignature* rootSig;
    hr = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
                                     IID_PPV_ARGS(&rootSig));
    if (FAILED(hr)) return nullptr;

    return rootSig;
}

ID3D12RootSignature* ME::RootSigDx::CreateRootSignature2DUIText(ID3D12Device* device) {
    if (!device) return nullptr;

    CD3DX12_ROOT_PARAMETER rootParameters[4];

    // Per Pass at register(b0)
    CD3DX12_DESCRIPTOR_RANGE cbvRangePass;
    cbvRangePass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    rootParameters[0].InitAsDescriptorTable(1, &cbvRangePass);

    // Texture Atlas Properties at register(b1)
    CD3DX12_DESCRIPTOR_RANGE cbvRangeAtlasProps;
    cbvRangeAtlasProps.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    rootParameters[1].InitAsDescriptorTable(1, &cbvRangeAtlasProps);

    // Texture at register(t0)
    CD3DX12_DESCRIPTOR_RANGE srvRangeTex;
    srvRangeTex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[2].InitAsDescriptorTable(1, &srvRangeTex);

    // Instance Buffer at register(t1)
    CD3DX12_DESCRIPTOR_RANGE srvRangeInstance;
    srvRangeInstance.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    rootParameters[3].InitAsDescriptorTable(1, &srvRangeInstance, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = GetStaticSamplerDescriptors();

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = _countof(rootParameters);
    desc.pParameters = rootParameters;
    desc.NumStaticSamplers = 6;
    desc.pStaticSamplers = staticSamplers;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* serialized;
    ID3DBlob* error;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        return nullptr;
    }

    ID3D12RootSignature* rootSig;
    hr = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
                                     IID_PPV_ARGS(&rootSig));
    if (FAILED(hr)) return nullptr;

    return rootSig;
}

ID3D12RootSignature* ME::RootSigDx::CreateRootSignature2DUISprite(ID3D12Device* device) {
    if (!device) return nullptr;

    CD3DX12_ROOT_PARAMETER rootParameters[4];

    // Per Pass at register(b0)
    CD3DX12_DESCRIPTOR_RANGE cbvRangePass;
    cbvRangePass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    rootParameters[0].InitAsDescriptorTable(1, &cbvRangePass);

    // Texture Atlas Properties at register(b1)
    CD3DX12_DESCRIPTOR_RANGE cbvRangeAtlasProps;
    cbvRangeAtlasProps.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    rootParameters[1].InitAsDescriptorTable(1, &cbvRangeAtlasProps);

    // Texture at register(t0)
    CD3DX12_DESCRIPTOR_RANGE srvRangeTex;
    srvRangeTex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[2].InitAsDescriptorTable(1, &srvRangeTex);

    // Instance Buffer at register(t1)
    CD3DX12_DESCRIPTOR_RANGE srvRangeInstance;
    srvRangeInstance.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    rootParameters[3].InitAsDescriptorTable(1, &srvRangeInstance, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = GetStaticSamplerDescriptors();

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = _countof(rootParameters);
    desc.pParameters = rootParameters;
    desc.NumStaticSamplers = 6;
    desc.pStaticSamplers = staticSamplers;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* serialized;
    ID3DBlob* error;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        return nullptr;
    }

    ID3D12RootSignature* rootSig;
    hr = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
                                     IID_PPV_ARGS(&rootSig));
    if (FAILED(hr)) return nullptr;

    return rootSig;
}

ID3D12RootSignature* ME::RootSigDx::CreateRootSignature3D(ID3D12Device* device) {
    if (!device) return nullptr;

    CD3DX12_ROOT_PARAMETER rootParameters[3];

    // Per Pass at register(b0)
    CD3DX12_DESCRIPTOR_RANGE cbvRangePass;
    cbvRangePass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    rootParameters[0].InitAsDescriptorTable(1, &cbvRangePass);

    // Per Object at register(b1)
    CD3DX12_DESCRIPTOR_RANGE cbvRangeObject;
    cbvRangeObject.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    rootParameters[1].InitAsDescriptorTable(1, &cbvRangeObject);

    // Texture at register(t0)
    CD3DX12_DESCRIPTOR_RANGE srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[2].InitAsDescriptorTable(1, &srvRange);

    CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = GetStaticSamplerDescriptors();

    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = _countof(rootParameters);
    desc.pParameters = rootParameters;
    desc.NumStaticSamplers = 6;
    desc.pStaticSamplers = staticSamplers;
    desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* serialized;
    ID3DBlob* error;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        return nullptr;
    }

    ID3D12RootSignature* rootSig;
    hr = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
                                     IID_PPV_ARGS(&rootSig));
    if (FAILED(hr)) return nullptr;

    return rootSig;
}

CD3DX12_STATIC_SAMPLER_DESC* ME::RootSigDx::GetStaticSamplerDescriptors() {
    static CD3DX12_STATIC_SAMPLER_DESC staticSamplers[6];

    // Point Wrap Sampler
    staticSamplers[0] =
        CD3DX12_STATIC_SAMPLER_DESC(0,  // register s0
                                    D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    0.0f,  // mipLODBias
                                    16,    // maxAnisotropy
                                    D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                                    0.0f,              // minLOD
                                    D3D12_FLOAT32_MAX  // maxLOD
        );

    // Point Clamp Sampler
    staticSamplers[1] =
        CD3DX12_STATIC_SAMPLER_DESC(1,  // register s1
                                    D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    0.0f,  // mipLODBias
                                    16,    // maxAnisotropy
                                    D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                                    0.0f,              // minLOD
                                    D3D12_FLOAT32_MAX  // maxLOD
        );
    // Linear Wrap Sampler
    staticSamplers[2] =
        CD3DX12_STATIC_SAMPLER_DESC(2,  // register s2
                                    D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    0.0f,  // mipLODBias
                                    16,    // maxAnisotropy
                                    D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                                    0.0f,              // minLOD
                                    D3D12_FLOAT32_MAX  // maxLOD
        );
    // Linear Clamp Sampler
    staticSamplers[3] =
        CD3DX12_STATIC_SAMPLER_DESC(3,  // register s3
                                    D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    0.0f,  // mipLODBias
                                    16,    // maxAnisotropy
                                    D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                                    0.0f,              // minLOD
                                    D3D12_FLOAT32_MAX  // maxLOD
        );
    // Anisotropic Wrap Sampler
    staticSamplers[4] =
        CD3DX12_STATIC_SAMPLER_DESC(4,  // register s4
                                    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    0.0f,  // mipLODBias
                                    16,    // maxAnisotropy
                                    D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                                    0.0f,              // minLOD
                                    D3D12_FLOAT32_MAX  // maxLOD
        );
    // Anisotropic Clamp Sampler
    staticSamplers[5] =
        CD3DX12_STATIC_SAMPLER_DESC(5,  // register s5
                                    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    0.0f,  // mipLODBias
                                    16,    // maxAnisotropy
                                    D3D12_COMPARISON_FUNC_ALWAYS, D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                                    0.0f,              // minLOD
                                    D3D12_FLOAT32_MAX  // maxLOD
        );

    return staticSamplers;
}

#endif  // ME_DX12
