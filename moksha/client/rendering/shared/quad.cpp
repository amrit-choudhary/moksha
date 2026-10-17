#include "quad.h"

ME::Quad::Quad() {
    vertices = new ME::Vertex2D[4];
    indices = new uint16_t[6];

    // Set up the vertices
    vertices[0].position = ME::Vec2{-0.5f, 0.5f};
    vertices[1].position = ME::Vec2{0.5f, 0.5f};
    vertices[2].position = ME::Vec2{0.5f, -0.5f};
    vertices[3].position = ME::Vec2{-0.5f, -0.5f};

    // Set up the texture coordinates
    vertices[0].uv = ME::Vec2{0.0f, 1.0f};
    vertices[1].uv = ME::Vec2{1.0f, 1.0f};
    vertices[2].uv = ME::Vec2{1.0f, 0.0f};
    vertices[3].uv = ME::Vec2{0.0f, 0.0f};

    // Set up the indices for two triangles
    indices[0] = 0;
    indices[1] = 2;
    indices[2] = 1;
    indices[3] = 0;
    indices[4] = 3;
    indices[5] = 2;

    vertexCount = 4;
    indexCount = 6;

    verticesSize = sizeof(ME::Vertex2D) * vertexCount;
    indicesSize = sizeof(uint16_t) * indexCount;
}

ME::Quad::~Quad() {
    delete[] vertices;
    delete[] indices;
}
