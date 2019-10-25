#pragma once

#include <Engine/Rendering/AssetContainers/AssetResources.hpp>
#include <d3d11.h>

// Not currently used
enum TX_TYPE {
    // Indicates a texture failed to load or has yet to be loaded
    NO_TEXTURE,
    DIFFUSE = aiTextureType_DIFFUSE,
    NORMAL = aiTextureType_NORMALS
};

struct Texture
{
    ID3D11ShaderResourceView* srv;
};
