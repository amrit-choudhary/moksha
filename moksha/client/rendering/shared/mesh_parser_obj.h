/**
 * Loads obj files and converts them to a common mesh format
 * that can be used by the different rendering engines.
 */

#pragma once

namespace ME {
class Mesh;

void CreateMeshFromOBJ(const char* fileName, Mesh& mesh);

}  // namespace ME
