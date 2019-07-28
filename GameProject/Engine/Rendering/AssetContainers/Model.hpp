#pragma once

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <d3d11.h>
#include <vector>

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 txCoords;
};

struct Mesh {
    ID3D11Buffer* vertexBuffer;
    unsigned int materialIndex;
};

struct Model {
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
};
