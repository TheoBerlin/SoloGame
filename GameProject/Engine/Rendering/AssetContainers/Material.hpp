#pragma once

#include <Engine/Rendering/AssetContainers/Texture.hpp>
#include <DirectXMath.h>

struct MaterialAttributes {
    DirectX::XMFLOAT3 ambient;
    DirectX::XMFLOAT3 specular;
    float shininess;
};

struct Material {
    std::vector<Texture> textures;
    MaterialAttributes attributes;
};
