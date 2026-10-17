#if ME_WIN

#include "renderer_dx.h"

#include <cmath>

#include "d3dx12.h"
#include "mesh_dx.h"
#include "moksha/client/misc/global_vars.h"
#include "moksha/client/rendering/shared/camera.h"
#include "moksha/client/rendering/shared/light.h"
#include "moksha/client/rendering/shared/mesh.h"
#include "moksha/client/rendering/shared/mesh_parser_obj.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/client/rendering/shared/texture.h"
#include "moksha/client/scene/scene_dx.h"
#include "moksha/client/scene/scene_ui_dx.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/math/transform.h"
#include "moksha/shared/math/vec16.h"
#include "moksha/shared/math/vec3.h"
#include "moksha/shared/misc/engine_limits.h"
#include "pso_dx.h"
#include "quad_dx.h"
#include "root_sig_dx.h"
#include "shader_dx.h"
#include "texture_dx.h"
#include "upload_buffer_dx.h"
#include "utils_dx.h"

void ME::RendererDX::Init() {
    ME::Log("RendererDirectX::Init");
}

void ME::RendererDX::SetScene(ME::Scene* gameScene) {
    // Delete the old scene if it exists.
    if (sceneDX != nullptr) {
        delete sceneDX;
    }

    // Keep ptr of the game scene to call update.
    this->scene = gameScene;

    // Do Initilization that need command list.
    directCmdListAlloc->Reset();
    commandList->Reset(directCmdListAlloc.Get(), pso2DAtl);

    // Create a new scene with the provided game scene.
    sceneDX = new ME::SceneDX(device.Get(), commandList.Get(), descHeapManager, gameScene);
    commandList->Close();
    ID3D12CommandList* cmdsLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();

    // TODO: Do Cleanup after upload is done.
    // scene->PostInitCleanup();
}

void ME::RendererDX::SetScenes(ME::Scene* gameScene, ME::SceneUI* uiScene) {
    // Do Initilization that need command list.
    directCmdListAlloc->Reset();
    commandList->Reset(directCmdListAlloc.Get(), pso2DAtl);

    if (sceneDX != nullptr) {
        delete sceneDX;
    }
    this->scene = gameScene;

    // Create a new scene with the provided game scene.
    sceneDX = new ME::SceneDX(device.Get(), commandList.Get(), descHeapManager, gameScene);

    if (uiSceneDX != nullptr) {
        delete uiSceneDX;
    }
    this->uiScene = uiScene;

    // Create a new UI scene with the provided UI scene.
    uiSceneDX = new ME::SceneUIDX(device.Get(), commandList.Get(), descHeapManager, uiScene);

    commandList->Close();
    ID3D12CommandList* cmdsLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();

    // TODO: Do Cleanup after upload is done.
    // scene->PostInitCleanup();
    // uiScene->PostInitCleanup();
}

void ME::RendererDX::SetDebugUIScene(ME::SceneUI* debugUiScene) {
    // Do Initilization that need command list.
    directCmdListAlloc->Reset();
    commandList->Reset(directCmdListAlloc.Get(), pso2DAtl);

    if (debugUiSceneDX != nullptr) {
        delete debugUiSceneDX;
    }
    this->debugUiScene = debugUiScene;

    // Create a new UI scene with the provided debug UI scene.
    debugUiSceneDX = new ME::SceneUIDX(device.Get(), commandList.Get(), descHeapManager, debugUiScene);

    commandList->Close();
    ID3D12CommandList* cmdsLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();
}

void ME::RendererDX::SetVsyncEnabled(bool enabled) {
    vsyncEnabled = enabled;
}

void ME::RendererDX::Update() {
    // If some of the scene render related data changed, update the scene.
    // Drawing is done after this.

    scene->Update();
    sceneDX->Update();

    // If UI scene is set, update it too.
    if (uiScene != nullptr) {
        uiScene->Update();
        uiSceneDX->Update();
    }

    // If the debug system's UI scene is set, update it too.
    if (debugUiScene != nullptr) {
        debugUiScene->Update();
        debugUiSceneDX->Update();
    }
}

void ME::RendererDX::End() {
    ME::Log("RendererDirectX::End");
}

bool ME::RendererDX::InitDX(HWND currenthWnd) {
#ifdef _DEBUG
    // Enable the D3D12 debug layer.
    ComPtr<ID3D12Debug> debugController;
    D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
    debugController->EnableDebugLayer();
#endif

    hWnd = currenthWnd;

    // Get window size from global vars.
    clientWidth = ME::GlobalVars::GetWindowWidth();
    clientHeight = ME::GlobalVars::GetWindowHeight();

    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        MessageBoxW(hWnd, L"Failed to create DXGI Factory", L"Error", MB_OK);
        return false;
    }

    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
        MessageBoxW(hWnd, L"Failed to create D3D12 Device", L"Error", MB_OK);
        return false;
    }

    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    cbvSrvUavDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue));

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&directCmdListAlloc));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, directCmdListAlloc.Get(), nullptr,
                              IID_PPV_ARGS(&commandList));
    commandList->Close();

    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.Width = clientWidth;
    scDesc.Height = clientHeight;
    scDesc.Format = swapChainFormat;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &scDesc, nullptr, nullptr,
                                    swapChain.ReleaseAndGetAddressOf());

    D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc;
    rtvDescHeapDesc.NumDescriptors = swapChainBufferCount;
    rtvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvDescHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&rtvDescHeapDesc, IID_PPV_ARGS(&rtvDescHeap));

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Format = backBufferFormat;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvDescHeap->GetCPUDescriptorHandleForHeapStart());
    for (size_t i = 0; i < swapChainBufferCount; ++i) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffers[i]));
        device->CreateRenderTargetView(swapChainBuffers[i].Get(), &rtvDesc, rtvHeapHandle);
        rtvHeapHandle.Offset(1, rtvDescriptorSize);
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvDescHeapDesc;
    dsvDescHeapDesc.NumDescriptors = 1;
    dsvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvDescHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&dsvDescHeapDesc, IID_PPV_ARGS(&dsvDescHeap));

    D3D12_RESOURCE_DESC depthStencilDesc{};
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = clientWidth;
    depthStencilDesc.Height = clientHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON,
                                    &clearValue, IID_PPV_ARGS(&depthStencilBuffer));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.Texture2D.MipSlice = 0;

    device->CreateDepthStencilView(depthStencilBuffer.Get(), &dsvDesc,
                                   dsvDescHeap->GetCPUDescriptorHandleForHeapStart());

    screenViewport.TopLeftX = 0;
    screenViewport.TopLeftY = 0;
    screenViewport.Width = static_cast<float>(clientWidth);
    screenViewport.Height = static_cast<float>(clientHeight);
    screenViewport.MinDepth = 0.0f;
    screenViewport.MaxDepth = 1.0f;

    scissorRect = {0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight)};

    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavDescHeapDesc{};
    // PerPass + some objects.
    cbvSrvUavDescHeapDesc.NumDescriptors = Constants::MaxDescriptorsOnDescriptorHeap;
    cbvSrvUavDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvUavDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvSrvUavDescHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&cbvSrvUavDescHeapDesc, IID_PPV_ARGS(&cbvSrvUavDescHeap));

    descHeapManager = new ME::DescHeapManagerDX(device.Get(), cbvSrvUavDescHeap.Get());

    // Create Root Signatures and PSOs.
    rootSig2DAtl = RootSigDx::CreateRootSignature2DAtlas(device.Get());
    pso2DAtl = PSODirectX::CreatePSO2DAtlas(device.Get(), "sprite_atlas.hlsl", rootSig2DAtl);
    rootSig2DInsAtl = RootSigDx::CreateRootSignature2DInstancedAtlas(device.Get());
    pso2DInsAtl = PSODirectX::CreatePSO2DInstancedAtlas(device.Get(), "sprite_instanced_atlas.hlsl", rootSig2DInsAtl);
    rootSig2DUIText = RootSigDx::CreateRootSignature2DUIText(device.Get());
    pso2DUIText = PSODirectX::CreatePSO2DUIText(device.Get(), "ui_text.hlsl", rootSig2DUIText);
    rootSig2DUISprite = RootSigDx::CreateRootSignature2DUISprite(device.Get());
    pso2DUISprite = PSODirectX::CreatePSO2DUISprite(device.Get(), "ui_sprite.hlsl", rootSig2DUISprite);

    // Do Initilization that need command list.
    directCmdListAlloc->Reset();
    commandList->Reset(directCmdListAlloc.Get(), pso2DAtl);
    CD3DX12_RESOURCE_BARRIER depthBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        depthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    commandList->ResourceBarrier(1, &depthBarrier);

    quad = new QuadDX{"quad", device.Get(), commandList.Get()};
    quad->CreateBuffers(device.Get(), commandList.Get());

    // End of Initilization that need command list.

    commandList->Close();
    ID3D12CommandList* cmdsLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    FlushCommandQueue();

    quad->ReleaseUploadBuffers();

    return true;
}

void ME::RendererDX::Draw() {
    directCmdListAlloc->Reset();
    commandList->Reset(directCmdListAlloc.Get(), pso2DAtl);

    commandList->SetPipelineState(pso2DAtl);
    commandList->SetGraphicsRootSignature(rootSig2DAtl);

    ID3D12DescriptorHeap* descriptorHeaps[] = {cbvSrvUavDescHeap.Get()};
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    commandList->RSSetViewports(1, &screenViewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    CD3DX12_RESOURCE_BARRIER presentToRenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        swapChainBuffers[currentBackBuffer].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &presentToRenderTargetBarrier);

    float clearColor[4] = {0.01f, 0.01f, 0.01f, 1.0f};

    commandList->ClearRenderTargetView(GetCurrentBackBufferHandle(), &clearColor[0], 0, nullptr);
    commandList->ClearDepthStencilView(GetDepthStencilHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f,
                                       0, 0, nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle = GetCurrentBackBufferHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle = GetDepthStencilHandle();
    commandList->OMSetRenderTargets(1, &backBufferHandle, true, &depthStencilHandle);

    ///////////////////////
    // Sprite Drawing.

    if (sceneDX->spriteRenderers.count != 0) {
        // Set PSO and Root Signature for non instanced sprites.
        commandList->SetPipelineState(pso2DAtl);
        commandList->SetGraphicsRootSignature(rootSig2DAtl);

        const uint32_t cbPerPassIndex = 0;

        CBPerPass perPassData{};
        perPassData.viewMatrix = sceneDX->spriteCamera->GetViewMatrix().GetDataRowMajor();
        perPassData.projectionMatrix = sceneDX->spriteCamera->GetProjectionMatrix().GetDataRowMajor();
        perPassData.ambientLightData = sceneDX->ambientLight->GetLightDataAmbient();
        perPassData.directionalLightData = sceneDX->directionalLight->GetLightDataDirectional();
        sceneDX->perPassCBs[cbPerPassIndex]->CopyData(&perPassData);
        D3D12_GPU_DESCRIPTOR_HANDLE cbvPerPass = descHeapManager->GetGPUDescriptorHandleForIndex(cbPerPassIndex);
        commandList->SetGraphicsRootDescriptorTable(0, cbvPerPass);

        D3D12_VERTEX_BUFFER_VIEW vbView = quad->GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibView = quad->GetIndexBufferView();
        commandList->IASetVertexBuffers(0, 1, &vbView);
        commandList->IASetIndexBuffer(&ibView);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        for (uint32_t i = 0; i < sceneDX->spriteRenderers.count; ++i) {
            const uint32_t textureIndex = sceneDX->spriteRenderers[i].textureId;
            const uint32_t atlasPropsIndex = sceneDX->spriteRenderers[i].textureAtlasPropsId;
            const uint32_t atlasPropsHeapIndex = sceneDX->textureAtlasCBHeapIndices[atlasPropsIndex];

            ME::TextureAtlasProperties atlasProps = sceneDX->textureAtlasProperties[atlasPropsIndex];
            sceneDX->textureAtlasCBs[atlasPropsIndex]->CopyData(&atlasProps);

            D3D12_GPU_DESCRIPTOR_HANDLE cbvAtlas = descHeapManager->GetGPUDescriptorHandleForIndex(atlasPropsHeapIndex);
            commandList->SetGraphicsRootDescriptorTable(1, cbvAtlas);

            D3D12_GPU_DESCRIPTOR_HANDLE textureHandle =
                descHeapManager->GetGPUDescriptorHandleForIndex(sceneDX->spriteTextures[textureIndex]->descHeapIndex);
            commandList->SetGraphicsRootDescriptorTable(2, textureHandle);

            CBPerSprite perSpriteData{};
            perSpriteData.modelMatrix = sceneDX->spriteTransforms[i].GetModelMatrix().GetDataRowMajor();
            perSpriteData.color = sceneDX->spriteRenderers[i].color;
            perSpriteData.atlasIndex = sceneDX->spriteRenderers[i].atlasIndex;
            perSpriteData.flags = sceneDX->spriteRenderers[i].flags;
            sceneDX->perSpriteCBs[i]->CopyData(&perSpriteData);

            D3D12_GPU_DESCRIPTOR_HANDLE cbvPerSprite =
                descHeapManager->GetGPUDescriptorHandleForIndex(sceneDX->perSpriteCBHeapIndices[i]);
            commandList->SetGraphicsRootDescriptorTable(3, cbvPerSprite);

            commandList->DrawIndexedInstanced(quad->indexCount, 1, 0, 0, 0);
        }
    }

    //////////////////////////////////
    // First Instanced Sprite Drawing.

    if (sceneDX->instancedSpriteRenderers0.count != 0) {
        // Set PSO and Root Signature for instanced sprites.
        commandList->SetPipelineState(pso2DInsAtl);
        commandList->SetGraphicsRootSignature(rootSig2DInsAtl);

        const uint32_t textureIndex = 1;
        const uint32_t atlasPropsIndex = 1;
        const uint32_t atlasPropsHeapIndex = sceneDX->textureAtlasCBHeapIndices[atlasPropsIndex];
        const uint32_t cbPerPassIndex = 0;

        CBPerPass perPassData{};
        perPassData.viewMatrix = sceneDX->spriteCamera->GetViewMatrix().GetDataRowMajor();
        perPassData.projectionMatrix = sceneDX->spriteCamera->GetProjectionMatrix().GetDataRowMajor();
        perPassData.ambientLightData = sceneDX->ambientLight->GetLightDataAmbient();
        perPassData.directionalLightData = sceneDX->directionalLight->GetLightDataDirectional();
        sceneDX->perPassCBs[cbPerPassIndex]->CopyData(&perPassData);

        D3D12_GPU_DESCRIPTOR_HANDLE cbvPerPass = descHeapManager->GetGPUDescriptorHandleForIndex(cbPerPassIndex);
        commandList->SetGraphicsRootDescriptorTable(0, cbvPerPass);

        ME::TextureAtlasProperties atlasProps = sceneDX->textureAtlasProperties[atlasPropsIndex];
        sceneDX->textureAtlasCBs[atlasPropsIndex]->CopyData(&atlasProps);

        D3D12_GPU_DESCRIPTOR_HANDLE cbvAtlas = descHeapManager->GetGPUDescriptorHandleForIndex(atlasPropsHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(1, cbvAtlas);

        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle =
            descHeapManager->GetGPUDescriptorHandleForIndex(sceneDX->spriteTextures[textureIndex]->descHeapIndex);

        commandList->SetGraphicsRootDescriptorTable(2, textureHandle);

        sceneDX->spriteInstanceBuffer0->CopyData(sceneDX->spriteInstanceData0);

        D3D12_GPU_DESCRIPTOR_HANDLE srvInstanceData =
            descHeapManager->GetGPUDescriptorHandleForIndex(sceneDX->spriteInstanceBufferHeapIndex0);
        commandList->SetGraphicsRootDescriptorTable(3, srvInstanceData);

        D3D12_VERTEX_BUFFER_VIEW vbView = quad->GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibView = quad->GetIndexBufferView();
        commandList->IASetVertexBuffers(0, 1, &vbView);
        commandList->IASetIndexBuffer(&ibView);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        commandList->DrawIndexedInstanced(quad->indexCount, sceneDX->instancedSpriteRenderers0.count, 0, 0, 0);
    }

    //////////////////////////////////
    // Second Instanced Sprite Drawing.

    if (sceneDX->instancedSpriteRenderers1.count != 0) {
        // Set PSO and Root Signature for instanced sprites.
        commandList->SetPipelineState(pso2DInsAtl);
        commandList->SetGraphicsRootSignature(rootSig2DInsAtl);

        const uint32_t textureIndex = 2;
        const uint32_t atlasPropsIndex = 2;
        const uint32_t atlasPropsHeapIndex = sceneDX->textureAtlasCBHeapIndices[atlasPropsIndex];
        const uint32_t cbPerPassIndex = 0;

        CBPerPass perPassData{};
        perPassData.viewMatrix = sceneDX->spriteCamera->GetViewMatrix().GetDataRowMajor();
        perPassData.projectionMatrix = sceneDX->spriteCamera->GetProjectionMatrix().GetDataRowMajor();
        perPassData.ambientLightData = sceneDX->ambientLight->GetLightDataAmbient();
        perPassData.directionalLightData = sceneDX->directionalLight->GetLightDataDirectional();
        sceneDX->perPassCBs[cbPerPassIndex]->CopyData(&perPassData);

        D3D12_GPU_DESCRIPTOR_HANDLE cbvPerPass = descHeapManager->GetGPUDescriptorHandleForIndex(cbPerPassIndex);
        commandList->SetGraphicsRootDescriptorTable(0, cbvPerPass);

        ME::TextureAtlasProperties atlasProps = sceneDX->textureAtlasProperties[atlasPropsIndex];
        sceneDX->textureAtlasCBs[atlasPropsIndex]->CopyData(&atlasProps);

        D3D12_GPU_DESCRIPTOR_HANDLE cbvAtlas = descHeapManager->GetGPUDescriptorHandleForIndex(atlasPropsHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(1, cbvAtlas);

        D3D12_GPU_DESCRIPTOR_HANDLE textureHandle =
            descHeapManager->GetGPUDescriptorHandleForIndex(sceneDX->spriteTextures[textureIndex]->descHeapIndex);

        commandList->SetGraphicsRootDescriptorTable(2, textureHandle);

        sceneDX->spriteInstanceBuffer1->CopyData(sceneDX->spriteInstanceData1);

        D3D12_GPU_DESCRIPTOR_HANDLE srvInstanceData =
            descHeapManager->GetGPUDescriptorHandleForIndex(sceneDX->spriteInstanceBufferHeapIndex1);
        commandList->SetGraphicsRootDescriptorTable(3, srvInstanceData);

        D3D12_VERTEX_BUFFER_VIEW vbView = quad->GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibView = quad->GetIndexBufferView();
        commandList->IASetVertexBuffers(0, 1, &vbView);
        commandList->IASetIndexBuffer(&ibView);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        commandList->DrawIndexedInstanced(quad->indexCount, sceneDX->instancedSpriteRenderers1.count, 0, 0, 0);
    }
    // End Sprite Drawing.

    ///////////////////////////
    // UI Drawing (game's own UI, then the debug system on top of it).
    DrawUIScene(uiSceneDX);

    DrawUIScene(debugUiSceneDX);

    /////////////////////
    // Drawing End

    CD3DX12_RESOURCE_BARRIER renderTargetToPresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        swapChainBuffers[currentBackBuffer].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &renderTargetToPresentBarrier);

    commandList->Close();

    ID3D12CommandList* cmdsLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    swapChain->Present(vsyncEnabled ? 1 : 0, 0);
    currentBackBuffer = (currentBackBuffer + 1) % swapChainBufferCount;

    FlushCommandQueue();
}

void ME::RendererDX::DrawUIScene(ME::SceneUIDX* uiSceneDX) {
    if (uiSceneDX == nullptr) {
        return;
    }

    ///////////////////////////
    // UI Sprite Drawing.
    if (uiSceneDX->uiSpriteRenderers.count != 0) {
        // Set PSO and Root Signature for instanced sprites.
        commandList->SetPipelineState(pso2DUISprite);
        commandList->SetGraphicsRootSignature(rootSig2DUISprite);

        const uint32_t textureIndex = 0;
        const uint32_t atlasPropsIndex = 0;
        const uint32_t atlasPropsHeapIndex = uiSceneDX->textureAtlasCBHeapIndices[atlasPropsIndex];
        const uint32_t cbPerPassIndex = 0;  // UI Sprite CB is at index 0.
        const uint32_t cbPerPassHeapIndex = uiSceneDX->perPassCBHeapIndices[cbPerPassIndex];

        CBPerPassUISprite perPassData{};
        perPassData.screenDimension = ME::GlobalVars::GetWindowSizeCombined();
        uiSceneDX->perPassCBs[cbPerPassIndex]->CopyData(&perPassData);
        D3D12_GPU_DESCRIPTOR_HANDLE cbvPerPass = descHeapManager->GetGPUDescriptorHandleForIndex(cbPerPassHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(0, cbvPerPass);

        ME::TextureAtlasProperties atlasPropsUISprite = uiSceneDX->textureAtlasProperties[atlasPropsIndex];
        uiSceneDX->textureAtlasCBs[atlasPropsIndex]->CopyData(&atlasPropsUISprite);

        D3D12_GPU_DESCRIPTOR_HANDLE cbvAtlasUISprite =
            descHeapManager->GetGPUDescriptorHandleForIndex(atlasPropsHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(1, cbvAtlasUISprite);

        D3D12_GPU_DESCRIPTOR_HANDLE textureHandleUISprite =
            descHeapManager->GetGPUDescriptorHandleForIndex(uiSceneDX->spriteTextures[textureIndex]->descHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(2, textureHandleUISprite);

        uiSceneDX->uiSpriteInstanceBuffer->CopyData(uiSceneDX->uiSpriteInstanceData.data);

        D3D12_GPU_DESCRIPTOR_HANDLE srvInstanceDataUISprite =
            descHeapManager->GetGPUDescriptorHandleForIndex(uiSceneDX->uiSpriteInstanceBufferHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(3, srvInstanceDataUISprite);

        D3D12_VERTEX_BUFFER_VIEW vbView = quad->GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibView = quad->GetIndexBufferView();
        commandList->IASetVertexBuffers(0, 1, &vbView);
        commandList->IASetIndexBuffer(&ibView);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        commandList->DrawIndexedInstanced(quad->indexCount, uiSceneDX->uiSpriteInstanceData.count, 0, 0, 0);
        // End Sprite Drawing.
    }

    ///////////////////////////
    // Start Text Drawing.
    if (uiSceneDX->textRenderers.count != 0) {
        // Set PSO and Root Signature for instanced sprites.
        commandList->SetPipelineState(pso2DUIText);
        commandList->SetGraphicsRootSignature(rootSig2DUIText);

        const uint32_t textureIndex = 1;
        const uint32_t atlasPropsIndex = 1;
        const uint32_t atlasPropsHeapIndex = uiSceneDX->textureAtlasCBHeapIndices[atlasPropsIndex];
        const uint32_t cbPerPassIndex = 1;  // UI Text CB is at index 1.
        const uint32_t cbPerPassHeapIndex = uiSceneDX->perPassCBHeapIndices[cbPerPassIndex];

        CBPerPassUIText perPassData{};
        perPassData.screenDimension = ME::GlobalVars::GetWindowSizeCombined();
        uiSceneDX->perPassCBs[cbPerPassIndex]->CopyData(&perPassData);
        D3D12_GPU_DESCRIPTOR_HANDLE cbvPerPass = descHeapManager->GetGPUDescriptorHandleForIndex(cbPerPassHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(0, cbvPerPass);

        ME::TextureAtlasProperties atlasPropsText = uiSceneDX->textureAtlasProperties[atlasPropsIndex];
        uiSceneDX->textureAtlasCBs[atlasPropsIndex]->CopyData(&atlasPropsText);

        D3D12_GPU_DESCRIPTOR_HANDLE cbvAtlasText = descHeapManager->GetGPUDescriptorHandleForIndex(atlasPropsHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(1, cbvAtlasText);

        D3D12_GPU_DESCRIPTOR_HANDLE textureHandleText =
            descHeapManager->GetGPUDescriptorHandleForIndex(uiSceneDX->spriteTextures[textureIndex]->descHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(2, textureHandleText);

        uiSceneDX->textInstanceBuffer->CopyData(uiSceneDX->textInstanceData.data);

        D3D12_GPU_DESCRIPTOR_HANDLE srvInstanceDataText =
            descHeapManager->GetGPUDescriptorHandleForIndex(uiSceneDX->textInstanceBufferHeapIndex);
        commandList->SetGraphicsRootDescriptorTable(3, srvInstanceDataText);

        D3D12_VERTEX_BUFFER_VIEW vbView = quad->GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibView = quad->GetIndexBufferView();
        commandList->IASetVertexBuffers(0, 1, &vbView);
        commandList->IASetIndexBuffer(&ibView);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        commandList->DrawIndexedInstanced(quad->indexCount, uiSceneDX->textInstanceData.count, 0, 0, 0);
    }
    // End Text Drawing.
}

void ME::RendererDX::FlushCommandQueue() {
    ++currentFence;
    commandQueue->Signal(fence.Get(), currentFence);
    if (fence->GetCompletedValue() < currentFence) {
        HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        fence->SetEventOnCompletion(currentFence, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE ME::RendererDX::GetCurrentBackBufferHandle() const {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescHeap->GetCPUDescriptorHandleForHeapStart(), currentBackBuffer,
                                         rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE ME::RendererDX::GetCurrentFrontBufferHandle() const {
    uint8_t currentFrontBuffer = (currentBackBuffer + 1) % swapChainBufferCount;
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescHeap->GetCPUDescriptorHandleForHeapStart(), currentFrontBuffer,
                                         rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE ME::RendererDX::GetDepthStencilHandle() const {
    return dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
}

#endif  // ME_WIN
