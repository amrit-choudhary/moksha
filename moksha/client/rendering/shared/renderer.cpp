#include "renderer.h"

#include <iostream>
#include <string>

#if defined(ME_DX12)
#include "moksha/client/rendering/directx/renderer_dx.h"
#elif defined(ME_METAL)
#include "moksha/client/rendering/metal/renderer_metal.h"
#else
#include "moksha/client/rendering/null/renderer_null.h"
#endif

ME::Renderer::Renderer() {}

ME::Renderer::~Renderer() {}

void ME::Renderer::Init() {
#if defined(ME_DX12)
    platformRenderer = new ME::RendererDX();
#elif defined(ME_METAL)
    platformRenderer = new ME::RendererMetal();
#else
    platformRenderer = new ME::RendererNull();
#endif
    platformRenderer->Init();
}

void ME::Renderer::Update() {
    platformRenderer->Update();
}

void ME::Renderer::End() {
    platformRenderer->End();
}

void ME::Renderer::SetVsyncEnabled(bool enabled) {
    platformRenderer->SetVsyncEnabled(enabled);
}
