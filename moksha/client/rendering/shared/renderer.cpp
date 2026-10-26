#include "renderer.h"

#include <iostream>
#include <string>

#ifdef ME_DX12
#include "moksha/client/rendering/directx/renderer_dx.h"
#else
#include "moksha/client/rendering/null/renderer_null.h"
#endif

ME::Renderer::Renderer() {}

ME::Renderer::~Renderer() {}

void ME::Renderer::Init() {
#ifdef ME_DX12
    platformRenderer = new ME::RendererDX();
#else
    // Headless, and Metal until its renderer is ported.
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
