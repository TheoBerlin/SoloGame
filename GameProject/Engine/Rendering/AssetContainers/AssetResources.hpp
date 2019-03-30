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
    // Indicates a texture failed to load or has yet to be loaded
    NO_TEXTURE,
    DIFFUSE = aiTextureType_DIFFUSE,
    NORMAL = aiTextureType_NORMALS
};

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 txCoords;
};
