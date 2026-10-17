/**
 * Mesh class for rendering 3D objects.
 */

#pragma once

#include <cstdint>

#include "moksha/shared/math/vec2.h"
#include "moksha/shared/math/vec3.h"
#include "moksha/shared/math/vec4.h"
#include "vertex.h"

namespace ME {
/**
 *
 */
class Mesh {
   public:
    Mesh();
    Mesh(const char* filename);
    virtual ~Mesh();

    void LoadFromFile(const char* filename);
    void Render();

    void CalculateNormal();

    ME::Vertex* vertices;  // Array of vertices that make up the mesh
    uint32_t* indices;     // Array of indices for indexed drawing

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    // Btye size of vertices and indices
    uint32_t verticesSize = 0;
    uint32_t indicesSize = 0;
};

}  // namespace ME
