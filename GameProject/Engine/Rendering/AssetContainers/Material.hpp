#pragma once

#include <Engine/Rendering/AssetContainers/Texture.hpp>
#include <DirectXMath.h>

struct Material {
    DirectX::XMFLOAT4 diffuse;
    DirectX::XMFLOAT4 specular;
    std::vector<Texture> textures;
};
