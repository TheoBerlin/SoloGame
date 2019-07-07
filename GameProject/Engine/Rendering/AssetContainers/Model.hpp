#pragma once

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <vector>

struct Mesh {
    std::vector<Vertex> vertices;
    unsigned int materialIndex;
};

struct Model {
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
};
