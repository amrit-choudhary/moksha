#include "mesh.h"

#include "mesh_parser_obj.h"

ME::Mesh::Mesh() {}

ME::Mesh::Mesh(const char* filename) {
    LoadFromFile(filename);
}

ME::Mesh::~Mesh() {
    delete[] vertices;
    delete[] indices;
}

void ME::Mesh::LoadFromFile(const char* filename) {
    ME::CreateMeshFromOBJ(filename, *this);
}

void ME::Mesh::Render() {}

void ME::Mesh::CalculateNormal() {
    for (int i = 0; i < indexCount - 3; i += 3) {
        int vertIndex = indices[i];
        ME::Vec3& v0 = vertices[indices[i]].position;
        ME::Vec3& v1 = vertices[indices[i + 1]].position;
        ME::Vec3& v2 = vertices[indices[i + 2]].position;
        ME::Vec3 normal = ME::Vec3::Cross(v1 - v0, v2 - v0);
        normal.Normalise();
        vertices[vertIndex].normal = normal;
    }
}
