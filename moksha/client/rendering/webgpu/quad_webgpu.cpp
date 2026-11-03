#ifdef ME_WEBGPU

#include "quad_webgpu.h"

ME::QuadWebGPU::~QuadWebGPU() {
    if (vertexBuffer != nullptr) {
        wgpuBufferRelease(vertexBuffer);
        vertexBuffer = nullptr;
    }
    if (indexBuffer != nullptr) {
        wgpuBufferRelease(indexBuffer);
        indexBuffer = nullptr;
    }
}

void ME::QuadWebGPU::CreateBuffers(WGPUDevice device, WGPUQueue queue) {
    WGPUBufferDescriptor descriptor = WGPU_BUFFER_DESCRIPTOR_INIT;
    descriptor.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    descriptor.size = verticesSize;
    vertexBuffer = wgpuDeviceCreateBuffer(device, &descriptor);
    wgpuQueueWriteBuffer(queue, vertexBuffer, 0, vertices, verticesSize);

    descriptor.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
    descriptor.size = indicesSize;
    indexBuffer = wgpuDeviceCreateBuffer(device, &descriptor);
    wgpuQueueWriteBuffer(queue, indexBuffer, 0, indices, indicesSize);
}

#endif  // ME_WEBGPU
