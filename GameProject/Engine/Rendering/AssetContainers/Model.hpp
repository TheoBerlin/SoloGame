#pragma once

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Texture.hpp>
#include <vector>

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 txCoords;
};

struct Mesh {
    std::vector<Vertex> vertices;
    unsigned int materialIndex;
    std::vector<Texture> textures;
};

struct Model {
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
};
