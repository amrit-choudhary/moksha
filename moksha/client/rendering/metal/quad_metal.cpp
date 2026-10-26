#ifdef ME_METAL

#include "quad_metal.h"

#include <Metal/Metal.hpp>

ME::QuadMetal::QuadMetal() : Quad() {}

ME::QuadMetal::~QuadMetal() {
    if (vertexBuffer != nullptr) {
        vertexBuffer->release();
        vertexBuffer = nullptr;
    }
    if (indexBuffer != nullptr) {
        indexBuffer->release();
        indexBuffer = nullptr;
    }
}

void ME::QuadMetal::CreateBuffers(MTL::Device* device) {
    vertexBuffer = device->newBuffer(vertices, verticesSize, MTL::ResourceStorageModeShared);
    indexBuffer = device->newBuffer(indices, indicesSize, MTL::ResourceStorageModeShared);
}

#endif  // ME_METAL
