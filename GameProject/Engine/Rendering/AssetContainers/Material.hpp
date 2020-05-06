#pragma once

#include <Engine/Rendering/APIAbstractions/Texture.hpp>

#include <DirectXMath.h>

struct MaterialAttributes {
    // R=shininess (exponent), G=shininess strength (factor), B,A = padding
    DirectX::XMFLOAT4 specular;
};

struct Material {
    std::vector<std::shared_ptr<Texture>> textures;
    MaterialAttributes attributes;
};
