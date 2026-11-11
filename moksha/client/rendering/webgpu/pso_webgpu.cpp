#ifdef ME_WEBGPU

#include "pso_webgpu.h"

#include <cstddef>

#include "moksha/client/rendering/shared/vertex.h"
#include "shader_webgpu.h"
#include "upload_buffer_webgpu.h"

namespace {

WGPUBindGroupLayoutEntry MakeBufferEntry(uint32_t binding, WGPUBufferBindingType type, bool hasDynamicOffset) {
    WGPUBindGroupLayoutEntry entry = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    entry.binding = binding;
    entry.visibility = WGPUShaderStage_Vertex;
    entry.buffer.type = type;
    entry.buffer.hasDynamicOffset = hasDynamicOffset;
    return entry;
}

WGPUBindGroupLayout CreateLayout(WGPUDevice device, const WGPUBindGroupLayoutEntry* entries, size_t entryCount) {
    WGPUBindGroupLayoutDescriptor descriptor = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    descriptor.entryCount = entryCount;
    descriptor.entries = entries;
    return wgpuDeviceCreateBindGroupLayout(device, &descriptor);
}

WGPUBindGroup CreateBindGroup(WGPUDevice device, WGPUBindGroupLayout layout, const WGPUBindGroupEntry* entries,
                              size_t entryCount) {
    WGPUBindGroupDescriptor descriptor = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
    descriptor.layout = layout;
    descriptor.entryCount = entryCount;
    descriptor.entries = entries;
    return wgpuDeviceCreateBindGroup(device, &descriptor);
}

WGPUBindGroupEntry MakeBufferBinding(uint32_t binding, WGPUBuffer buffer, uint64_t size) {
    WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
    entry.binding = binding;
    entry.buffer = buffer;
    entry.size = size;
    return entry;
}

}  // namespace

WGPUBindGroupLayout ME::PSOWebGPU::CreatePassLayout(WGPUDevice device) {
    const WGPUBindGroupLayoutEntry entries[] = {MakeBufferEntry(0, WGPUBufferBindingType_Uniform, false),
                                                MakeBufferEntry(1, WGPUBufferBindingType_Uniform, true)};
    return CreateLayout(device, entries, 2);
}

WGPUBindGroupLayout ME::PSOWebGPU::CreateTextureLayout(WGPUDevice device) {
    WGPUBindGroupLayoutEntry entries[2] = {WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT, WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT};
    entries[0].binding = 0;
    entries[0].visibility = WGPUShaderStage_Fragment;
    entries[0].texture.sampleType = WGPUTextureSampleType_Float;
    entries[0].texture.viewDimension = WGPUTextureViewDimension_2D;
    entries[1].binding = 1;
    entries[1].visibility = WGPUShaderStage_Fragment;
    entries[1].sampler.type = WGPUSamplerBindingType_Filtering;
    return CreateLayout(device, entries, 2);
}

WGPUBindGroupLayout ME::PSOWebGPU::CreatePerSpriteLayout(WGPUDevice device) {
    const WGPUBindGroupLayoutEntry entry = MakeBufferEntry(0, WGPUBufferBindingType_Uniform, true);
    return CreateLayout(device, &entry, 1);
}

WGPUBindGroupLayout ME::PSOWebGPU::CreateInstanceLayout(WGPUDevice device) {
    const WGPUBindGroupLayoutEntry entry = MakeBufferEntry(0, WGPUBufferBindingType_ReadOnlyStorage, false);
    return CreateLayout(device, &entry, 1);
}

WGPUPipelineLayout ME::PSOWebGPU::CreatePipelineLayout(WGPUDevice device, WGPUBindGroupLayout passLayout,
                                                       WGPUBindGroupLayout textureLayout,
                                                       WGPUBindGroupLayout objectLayout) {
    const WGPUBindGroupLayout layouts[] = {passLayout, textureLayout, objectLayout};
    WGPUPipelineLayoutDescriptor descriptor = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
    descriptor.bindGroupLayoutCount = 3;
    descriptor.bindGroupLayouts = layouts;
    return wgpuDeviceCreatePipelineLayout(device, &descriptor);
}

WGPURenderPipeline ME::PSOWebGPU::CreatePSO2D(WGPUDevice device, WGPUPipelineLayout layout, const char* shaderName,
                                              WGPUTextureFormat colorFormat, bool depthTest) {
    ME::ShaderWebGPU shader(device, shaderName);
    if (shader.GetModule() == nullptr) {
        return nullptr;
    }

    // POSITION and TEXCOORD, two float2s per Vertex2D.
    WGPUVertexAttribute attributes[2] = {WGPU_VERTEX_ATTRIBUTE_INIT, WGPU_VERTEX_ATTRIBUTE_INIT};
    attributes[0].format = WGPUVertexFormat_Float32x2;
    attributes[0].offset = 0;
    attributes[0].shaderLocation = 0;
    attributes[1].format = WGPUVertexFormat_Float32x2;
    attributes[1].offset = 8;
    attributes[1].shaderLocation = 1;

    WGPUVertexBufferLayout vertexBuffer = WGPU_VERTEX_BUFFER_LAYOUT_INIT;
    vertexBuffer.stepMode = WGPUVertexStepMode_Vertex;
    vertexBuffer.arrayStride = sizeof(ME::Vertex2D);
    vertexBuffer.attributeCount = 2;
    vertexBuffer.attributes = attributes;

    // Alpha blending as in the other backends: src alpha / inverse src alpha for colour, one / zero for alpha.
    WGPUBlendState blend = WGPU_BLEND_STATE_INIT;
    blend.color.operation = WGPUBlendOperation_Add;
    blend.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blend.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blend.alpha.operation = WGPUBlendOperation_Add;
    blend.alpha.srcFactor = WGPUBlendFactor_One;
    blend.alpha.dstFactor = WGPUBlendFactor_Zero;

    WGPUColorTargetState colorTarget = WGPU_COLOR_TARGET_STATE_INIT;
    colorTarget.format = colorFormat;
    colorTarget.blend = &blend;
    colorTarget.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragment = WGPU_FRAGMENT_STATE_INIT;
    fragment.module = shader.GetModule();
    fragment.entryPoint = {"PS", WGPU_STRLEN};
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;

    WGPUDepthStencilState depth = WGPU_DEPTH_STENCIL_STATE_INIT;
    depth.format = DepthFormat;
    depth.depthWriteEnabled = depthTest ? WGPUOptionalBool_True : WGPUOptionalBool_False;
    depth.depthCompare = depthTest ? WGPUCompareFunction_Less : WGPUCompareFunction_Always;

    WGPURenderPipelineDescriptor descriptor = WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;
    descriptor.label = {shaderName, WGPU_STRLEN};
    descriptor.layout = layout;
    descriptor.vertex.module = shader.GetModule();
    descriptor.vertex.entryPoint = {"VS", WGPU_STRLEN};
    descriptor.vertex.bufferCount = 1;
    descriptor.vertex.buffers = &vertexBuffer;
    // Counter-clockwise front faces, nothing culled: the DX 2D rasterizer state (a sprite mirrored by a negative
    // scale still draws).
    descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    descriptor.primitive.frontFace = WGPUFrontFace_CCW;
    descriptor.primitive.cullMode = WGPUCullMode_None;
    descriptor.depthStencil = &depth;
    descriptor.fragment = &fragment;
    return wgpuDeviceCreateRenderPipeline(device, &descriptor);
}

WGPUBindGroup ME::PSOWebGPU::CreatePassBindGroup(const WebGPUContext& context, const UploadBufferWebGPU* perPassBuffer,
                                                 const UploadBufferWebGPU* atlasBuffer) {
    const WGPUBindGroupEntry entries[] = {
        MakeBufferBinding(0, perPassBuffer->GetBuffer(), perPassBuffer->GetElementSize()),
        MakeBufferBinding(1, atlasBuffer->GetBuffer(), atlasBuffer->GetElementSize())};
    return CreateBindGroup(context.device, context.passLayout, entries, 2);
}

WGPUBindGroup ME::PSOWebGPU::CreateTextureBindGroup(const WebGPUContext& context, WGPUTextureView view) {
    if (view == nullptr) {
        return nullptr;
    }
    WGPUBindGroupEntry entries[2] = {WGPU_BIND_GROUP_ENTRY_INIT, WGPU_BIND_GROUP_ENTRY_INIT};
    entries[0].binding = 0;
    entries[0].textureView = view;
    entries[1].binding = 1;
    entries[1].sampler = context.sampler;
    return CreateBindGroup(context.device, context.textureLayout, entries, 2);
}

WGPUBindGroup ME::PSOWebGPU::CreatePerSpriteBindGroup(const WebGPUContext& context, const UploadBufferWebGPU* buffer) {
    const WGPUBindGroupEntry entry = MakeBufferBinding(0, buffer->GetBuffer(), buffer->GetElementSize());
    return CreateBindGroup(context.device, context.perSpriteLayout, &entry, 1);
}

WGPUBindGroup ME::PSOWebGPU::CreateInstanceBindGroup(const WebGPUContext& context, const UploadBufferWebGPU* buffer) {
    const WGPUBindGroupEntry entry = MakeBufferBinding(0, buffer->GetBuffer(), WGPU_WHOLE_SIZE);
    return CreateBindGroup(context.device, context.instanceLayout, &entry, 1);
}

#endif  // ME_WEBGPU
