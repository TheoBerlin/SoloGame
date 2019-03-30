#pragma once

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <vector>

class Mesh
{
public:
    Mesh();
    ~Mesh();

    std::vector<Vertex>& getVertices();

    void allocateVertices(size_t vertexCount);

    unsigned int getMaterialIndex() const;
    void setMaterialIndex(unsigned int materialIndex);

private:
    std::vector<Vertex> vertices;

    unsigned int materialIndex;
};
