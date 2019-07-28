#pragma once

#include <Engine/Rendering/AssetContainers/AssetResources.hpp>
#include <d3d11.h>

enum TX_TYPE {
    // Indicates a texture failed to load or has yet to be loaded
    NO_TEXTURE,
    DIFFUSE = aiTextureType_DIFFUSE,
    NORMAL = aiTextureType_NORMALS
};

struct Texture
{
    TX_TYPE type;
    ID3D11ShaderResourceView* srv;
};
