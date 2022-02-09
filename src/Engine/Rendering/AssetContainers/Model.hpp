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
    IBuffer* pVertexBuffer, *pIndexBuffer;
    size_t vertexCount, indexCount, materialIndex;
};

struct Model {
    std::vector<Mesh> Meshes;
    std::vector<Material> Materials;
};

struct ModelComponent {
    DECL_COMPONENT(ModelComponent);
    std::shared_ptr<Model> ModelPtr;
};

inline void ReleaseModel(Model* pModel)
{
    for (Mesh& mesh : pModel->Meshes) {
        delete mesh.pVertexBuffer;
        delete mesh.pIndexBuffer;
    }

    delete pModel;
}
