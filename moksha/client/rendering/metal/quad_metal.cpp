#ifdef ME_METAL

#include "quad_metal.h"

ME::QuadMetal::QuadMetal(const char* path, MTL::Device* device, MTL::CommandQueue* cmdQueue) : Quad() {
    vertexBuffer = device->newBuffer(verticesSize, MTL::ResourceStorageModeManaged);
    indexBuffer = device->newBuffer(indicesSize, MTL::ResourceStorageModeManaged);

    memcpy(vertexBuffer->contents(), vertices, verticesSize);
    vertexBuffer->didModifyRange(NS::Range::Make(0, vertexBuffer->length()));
    memcpy(indexBuffer->contents(), indices, indicesSize);
    indexBuffer->didModifyRange(NS::Range::Make(0, indexBuffer->length()));
}

ME::QuadMetal::~QuadMetal() {
    vertexBuffer->release();
    indexBuffer->release();
}

#endif
