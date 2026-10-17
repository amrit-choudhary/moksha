#pragma once
#ifdef ME_WIN

/**
 * Renderer class for DirectX 12 on Windows.
 */

#include <d3d12.h>
#include <dxgi1_6.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <wrl.h>

#include "desc_heap_manager_dx.h"
#include "moksha/client/rendering/shared/renderer.h"

using Microsoft::WRL::ComPtr;

namespace ME {

// Forward declarations to avoid including shader/quad headers here.
class Scene;
class SceneDX;
class SceneUI;
class SceneUIDX;
class QuadDX;
class WindowState;

/**
 * Renderer class for DirectX 12 on Windows.
 */
class RendererDX : public PlatformRenderer {
   public:
    // Init game
    void Init() override;

    // Sets scene for the renderer.
    void SetScene(ME::Scene* gameScene) override;

    // Sets both game and UI scenes for the renderer.
    void SetScenes(ME::Scene* gameScene, ME::SceneUI* uiScene) override;

    // Sets the debug system's UI scene, drawn in its own pass after the main UI scene.
    void SetDebugUIScene(ME::SceneUI* debugUiScene) override;

    // Will return true if game should tick based on FFR.
    void Update() override;

    // Will stop the timers to let it calculate average FPS.
    void End() override;

    // Initialize DirectX 12 for hwnd. window is the app's window state; its size sizes the swap chain.
    bool InitDX(HWND hwnd, const WindowState* inWindow);

    void Draw();

    void SetVsyncEnabled(bool enabled) override;

   private:
    bool vsyncEnabled = true;

    ME::Scene* scene = nullptr;
    ME::SceneDX* sceneDX = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUIDX* uiSceneDX = nullptr;
    ME::SceneUI* debugUiScene = nullptr;
    ME::SceneUIDX* debugUiSceneDX = nullptr;

    HWND hWnd;
    const WindowState* window = nullptr;
    ComPtr<IDXGIFactory4> factory;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<IDXGISwapChain1> swapChain;
    ComPtr<ID3D12Fence> fence;
    uint64_t currentFence = 0;

    ComPtr<ID3D12CommandAllocator> directCmdListAlloc;
    ComPtr<ID3D12GraphicsCommandList> commandList;

    static const uint8_t swapChainBufferCount = 2;
    uint8_t currentBackBuffer = 0;
    ComPtr<ID3D12Resource> swapChainBuffers[swapChainBufferCount];
    ComPtr<ID3D12Resource> depthStencilBuffer;

    D3D12_VIEWPORT screenViewport;
    D3D12_RECT scissorRect;

    // PSO for 2D rendering with texture atlases. No instancing.
    ID3D12PipelineState* pso2DAtl = nullptr;
    // Root signature for 2D rendering with texture atlases. No instancing.
    ID3D12RootSignature* rootSig2DAtl = nullptr;
    // PSO for 2D rendering with instancing and texture atlases.
    ID3D12PipelineState* pso2DInsAtl = nullptr;
    // Root signature for 2D rendering with instancing and texture atlases.
    ID3D12RootSignature* rootSig2DInsAtl = nullptr;
    // PSO for UI Text rendering.
    ID3D12PipelineState* pso2DUIText = nullptr;
    // Root signature for UI Text rendering.
    ID3D12RootSignature* rootSig2DUIText = nullptr;
    // PSO for UI Sprite rendering.
    ID3D12PipelineState* pso2DUISprite = nullptr;
    // Root signature for UI Sprite rendering.
    ID3D12RootSignature* rootSig2DUISprite = nullptr;

    uint32_t rtvDescriptorSize = 0;
    uint32_t dsvDescriptorSize = 0;
    uint32_t cbvSrvUavDescriptorSize = 0;

    ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
    ComPtr<ID3D12DescriptorHeap> dsvDescHeap;
    ComPtr<ID3D12DescriptorHeap> cbvSrvUavDescHeap;

    D3D_DRIVER_TYPE d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;  // DX12 reasons for UNORM.
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D32_FLOAT;
    uint32_t clientWidth;
    uint32_t clientHeight;

    void FlushCommandQueue();

    // Issues the UI Sprite + UI Text draw passes for one SceneUIDX. Shared by the main UI scene
    // and the debug system's UI scene — same PSOs, same draw commands, different data. No-op if
    // uiSceneDX is nullptr.
    void DrawUIScene(ME::SceneUIDX* uiSceneDX);

    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferHandle() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentFrontBufferHandle() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilHandle() const;

    ME::DescHeapManagerDX* descHeapManager = nullptr;

    ME::QuadDX* quad = nullptr;
};

}  // namespace ME

#endif  // ME_WIN
