#pragma once

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>

#include <DirectXMath.h>
#include <vector>

struct Vertex2D {
    DirectX::XMFLOAT2 Position;
    DirectX::XMFLOAT2 TXCoords;
};

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 txCoords;
};

struct Mesh {
    BufferDX11* pVertexBuffer, *pIndexBuffer;
    size_t vertexCount, indexCount, materialIndex;
};

struct Model {
    std::vector<Mesh> Meshes;
    std::vector<Material> Materials;
};

inline void releaseModel(Model* pModel)
{
    for (Mesh& mesh : pModel->Meshes) {
        delete mesh.pVertexBuffer;
        delete mesh.pIndexBuffer;
    }
}
