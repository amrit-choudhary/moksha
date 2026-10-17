#ifdef ME_METAL

#include "mesh_metal.h"

ME::MeshMetal::MeshMetal(const char* path, MTL::Device* device, MTL::CommandQueue* cmdQueue) {
    LoadFromFile(path);
    vertexBuffer = device->newBuffer(verticesSize, MTL::ResourceStorageModeManaged);
    indexBuffer = device->newBuffer(indicesSize, MTL::ResourceStorageModeManaged);

    memcpy(vertexBuffer->contents(), vertices, verticesSize);
    vertexBuffer->didModifyRange(NS::Range::Make(0, vertexBuffer->length()));
    memcpy(indexBuffer->contents(), indices, indicesSize);
    indexBuffer->didModifyRange(NS::Range::Make(0, indexBuffer->length()));
}

ME::MeshMetal::~MeshMetal() {
    vertexBuffer->release();
    indexBuffer->release();
}

#endif
