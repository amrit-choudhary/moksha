#include "mesh_parser_obj.h"

#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "mesh.h"
#include "moksha/shared/misc/utils.h"

void ME::CreateMeshFromOBJ(const char* fileName, ME::Mesh& mesh) {
    std::string filePath = ME::Utils::GetResourcesPath() + fileName;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    std::vector<ME::Vertex> vertices;
    std::vector<uint32_t> indices;

    std::vector<ME::Vec3> tempPositions;
    std::vector<ME::Vec3> tempNormals;
    std::vector<ME::Vec2> tempUVs;
    std::vector<uint32_t> tempNormalIndices;
    std::vector<uint32_t> tempUVIndices;

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            tempPositions.emplace_back(x, y, z);
        } else if (type == "vn") {
            float x, y, z;
            iss >> x >> y >> z;
            tempNormals.emplace_back(x, y, z);
        } else if (type == "vt") {
            float u, v;
            iss >> u >> v;
            tempUVs.emplace_back(u, v);
        } else if (type == "f") {
            std::string vertex;
            for (int i = 0; i < 3; ++i) {
                iss >> vertex;
                std::istringstream vss(vertex);
                uint32_t v_idx, vt_idx, vn_idx;
                char slash;
                vss >> v_idx >> slash >> vt_idx >> slash >> vn_idx;
                indices.emplace_back(v_idx - 1);
                tempUVIndices.emplace_back(vt_idx - 1);
                tempNormalIndices.emplace_back(vn_idx - 1);
            }
        }
    }
    file.close();

    for (int i = 0; i < tempPositions.size(); ++i) {
        Vertex vertex;
        vertex.position = tempPositions[i];
        vertices.emplace_back(vertex);
    }

    for (int i = 0; i < indices.size(); ++i) {
        uint32_t index = indices[i];
        vertices[index].normal = tempNormals[tempNormalIndices[i]];
        vertices[index].uv = tempUVs[tempUVIndices[i]];
    }

    mesh.vertexCount = vertices.size();
    mesh.indexCount = indices.size();
    mesh.verticesSize = mesh.vertexCount * sizeof(ME::Vertex);
    mesh.indicesSize = mesh.indexCount * sizeof(uint32_t);

    mesh.vertices = new ME::Vertex[mesh.vertexCount];
    for (uint32_t i = 0; i < mesh.vertexCount; ++i) {
        mesh.vertices[i] = vertices[i];
    }
    mesh.indices = new uint32_t[mesh.indexCount];
    for (uint32_t i = 0; i < mesh.indexCount; ++i) {
        mesh.indices[i] = indices[i];
    }
}
