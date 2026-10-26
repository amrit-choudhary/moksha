#ifdef ME_METAL

#include "pso_metal.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>

#include "moksha/client/rendering/shared/vertex.h"
#include "moksha/logging/logging.h"
#include "shader_metal.h"

namespace {

// The window's back buffer format: sRGB like DX12's R8G8B8A8_UNORM_SRGB, in the order CAMetalLayer uses.
constexpr MTL::PixelFormat kBackBufferFormat = MTL::PixelFormatBGRA8Unorm_sRGB;

MTL4::LibraryFunctionDescriptor* MakeFunction(MTL::Library* library, const char* name) {
    MTL4::LibraryFunctionDescriptor* function = MTL4::LibraryFunctionDescriptor::alloc()->init();
    function->setLibrary(library);
    function->setName(NS::String::string(name, NS::UTF8StringEncoding));
    return function;
}

// The DX input layout of every 2D PSO: POSITION and TEXCOORD, two float2s per Vertex2D.
MTL::VertexDescriptor* MakeVertexDescriptor2D() {
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    MTL::VertexAttributeDescriptor* position = vertexDescriptor->attributes()->object(0);
    position->setFormat(MTL::VertexFormatFloat2);
    position->setOffset(0);
    position->setBufferIndex(ME::MetalBindings::VertexBuffer);
    MTL::VertexAttributeDescriptor* uv = vertexDescriptor->attributes()->object(1);
    uv->setFormat(MTL::VertexFormatFloat2);
    uv->setOffset(8);
    uv->setBufferIndex(ME::MetalBindings::VertexBuffer);
    vertexDescriptor->layouts()->object(ME::MetalBindings::VertexBuffer)->setStride(sizeof(ME::Vertex2D));
    return vertexDescriptor;
}

// Alpha blending as in the DX sprite and UI PSOs: src alpha / inverse src alpha for colour, one / zero for alpha.
void SetAlphaBlend(MTL4::RenderPipelineColorAttachmentDescriptor* target) {
    target->setPixelFormat(kBackBufferFormat);
    target->setBlendingState(MTL4::BlendStateEnabled);
    target->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    target->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    target->setRgbBlendOperation(MTL::BlendOperationAdd);
    target->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    target->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);
    target->setAlphaBlendOperation(MTL::BlendOperationAdd);
}

// A 2D atlas pipeline from shaderName's VS and PS, alpha blended.
MTL::RenderPipelineState* CreatePSO2DBlended(MTL4::Compiler* compiler, const char* shaderName) {
    ME::ShaderMetal shader(compiler, shaderName);
    if (shader.GetLibrary() == nullptr) {
        return nullptr;
    }

    MTL4::LibraryFunctionDescriptor* vertexFunction = MakeFunction(shader.GetLibrary(), "VS");
    MTL4::LibraryFunctionDescriptor* fragmentFunction = MakeFunction(shader.GetLibrary(), "PS");
    MTL::VertexDescriptor* vertexDescriptor = MakeVertexDescriptor2D();

    MTL4::RenderPipelineDescriptor* descriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
    descriptor->setVertexFunctionDescriptor(vertexFunction);
    descriptor->setFragmentFunctionDescriptor(fragmentFunction);
    descriptor->setVertexDescriptor(vertexDescriptor);
    descriptor->setInputPrimitiveTopology(MTL::PrimitiveTopologyClassTriangle);
    SetAlphaBlend(descriptor->colorAttachments()->object(0));

    NS::Error* error = nullptr;
    MTL::RenderPipelineState* pso = compiler->newRenderPipelineState(descriptor, nullptr, &error);
    if (pso == nullptr) {
        ME::LogError("PSOMetal: ", shaderName, " pipeline failed: ",
                     error != nullptr ? error->localizedDescription()->utf8String() : "unknown error");
    }

    descriptor->release();
    vertexDescriptor->release();
    fragmentFunction->release();
    vertexFunction->release();
    return pso;
}

}  // namespace

MTL::RenderPipelineState* ME::PSOMetal::CreatePSO2DAtlas(MTL4::Compiler* compiler, const char* shaderName) {
    return CreatePSO2DBlended(compiler, shaderName);
}

MTL::RenderPipelineState* ME::PSOMetal::CreatePSO2DInstancedAtlas(MTL4::Compiler* compiler, const char* shaderName) {
    return CreatePSO2DBlended(compiler, shaderName);
}

MTL::RenderPipelineState* ME::PSOMetal::CreatePSO2DUISprite(MTL4::Compiler* compiler, const char* shaderName) {
    return CreatePSO2DBlended(compiler, shaderName);
}

MTL::RenderPipelineState* ME::PSOMetal::CreatePSO2DUIText(MTL4::Compiler* compiler, const char* shaderName) {
    return CreatePSO2DBlended(compiler, shaderName);
}

MTL::DepthStencilState* ME::PSOMetal::CreateDepthStateDefault(MTL::Device* device) {
    MTL::DepthStencilDescriptor* descriptor = MTL::DepthStencilDescriptor::alloc()->init();
    descriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
    descriptor->setDepthWriteEnabled(true);
    MTL::DepthStencilState* state = device->newDepthStencilState(descriptor);
    descriptor->release();
    return state;
}

MTL::DepthStencilState* ME::PSOMetal::CreateDepthStateDisabled(MTL::Device* device) {
    MTL::DepthStencilDescriptor* descriptor = MTL::DepthStencilDescriptor::alloc()->init();
    descriptor->setDepthCompareFunction(MTL::CompareFunctionAlways);
    descriptor->setDepthWriteEnabled(false);
    MTL::DepthStencilState* state = device->newDepthStencilState(descriptor);
    descriptor->release();
    return state;
}

#endif  // ME_METAL
