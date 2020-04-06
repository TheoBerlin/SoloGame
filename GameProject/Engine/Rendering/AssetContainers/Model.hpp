#pragma once

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <d3d11.h>
#include <vector>

struct Vertex2D {
    DirectX::XMFLOAT2 position;
    DirectX::XMFLOAT2 txCoords;
};

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 txCoords;
};

struct Mesh {
    ID3D11Buffer* vertexBuffer, *indexBuffer;
    size_t vertexCount, indexCount, materialIndex;
};

struct Model {
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
};

inline void releaseModel(Model* pModel)
{
    for (size_t i = 0; i < pModel->meshes.size(); i += 1) {
        pModel->meshes[i].vertexBuffer->Release();
        pModel->meshes[i].indexBuffer->Release();
    }
}
