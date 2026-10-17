#pragma once

/**
 * Quad class for rendering 2D.
 * Based on the Mesh class.
 */

#include <cstdint>

#include "moksha/shared/math/vec2.h"
#include "vertex.h"

namespace ME {

/**
 * Quad class for rendering 2D.
 * It has two triangles that make up a quad.
 * Origin is at top left corner. Size is 1x1.
 * 0-----1
 * |     |
 * 3-----2
 */
class Quad {
   public:
    Quad();
    virtual ~Quad();

    ME::Vertex2D* vertices;  // Array of vertices that make up the quad
    uint16_t* indices;       // Array of indices for indexed drawing, Metal suport 16 or 32 bit indices.

    uint8_t vertexCount = 0;
    uint8_t indexCount = 0;

    // Btye size of vertices and indices
    uint8_t verticesSize = 0;
    uint8_t indicesSize = 0;
};

}  // namespace ME
