#pragma once

/*
    Containers that do not require their own logic (functions) are defined
    as structs here
*/

#include <assimp/material.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

enum TX_TYPE {
    // Indicates a texture failed to load
    NO_TEXTURE,
    DIFFUSE = aiTextureType_DIFFUSE,
    NORMAL = aiTextureType_NORMALS
};

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 txCoords;
};

struct Texture {
    TX_TYPE txType;
    ID3D11ShaderResourceView* srv;
};

struct Material {
    DirectX::XMFLOAT4 diffuse;
    DirectX::XMFLOAT4 specular;
    std::vector<Texture> textures;
};
