#include "renderer.h"

#include <iostream>
#include <string>

#include "moksha/client/rendering/directx/renderer_dx.h"
#include "moksha/client/rendering/metal/renderer_metal.h"

ME::Renderer::Renderer() {}

ME::Renderer::~Renderer() {}

void ME::Renderer::Init() {
#ifdef ME_MAC
    platformRenderer = new ME::RendererMetal();
#elif ME_WIN
    platformRenderer = new ME::RendererDX();
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
