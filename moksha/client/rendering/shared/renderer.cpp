#include "renderer.h"

#include <iostream>
#include <string>

#ifdef ME_HEADLESS
#include "moksha/client/rendering/null/renderer_null.h"
#else
#include "moksha/client/rendering/directx/renderer_dx.h"
#include "moksha/client/rendering/metal/renderer_metal.h"
#endif

ME::Renderer::Renderer() {}

ME::Renderer::~Renderer() {}

void ME::Renderer::Init() {
#if defined(ME_HEADLESS)
    platformRenderer = new ME::RendererNull();
#elif defined(ME_MAC)
    platformRenderer = new ME::RendererMetal();
#elif defined(ME_WIN)
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
