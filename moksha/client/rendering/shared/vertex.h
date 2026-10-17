/**
 * Vertex class representing a 3D vertex with position, normal, and texture coordinates.
 * This class is used in 3D rendering and graphics applications.
 */

#pragma once

#include "moksha/shared/math/vec2.h"
#include "moksha/shared/math/vec3.h"

namespace ME {

class Vertex {
   public:
    ME::Vec3 position;  // Position of the vertex in 3D space.
    ME::Vec3 normal;    // Normal vector at the vertex for lighting calculations.
    ME::Vec2 uv;        // Texture coordinates for mapping textures to the vertex.
   private:
};

class Vertex2D {
   public:
    ME::Vec2 position;  // Position of the vertex in 2D space.
    ME::Vec2 uv;        // Texture coordinates for mapping textures to the vertex.
   private:
};

}  // namespace ME
