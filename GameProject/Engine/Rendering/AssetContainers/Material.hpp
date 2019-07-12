#pragma once

#include <DirectXMath.h>

struct Material {
    DirectX::XMFLOAT3 ambient;
    DirectX::XMFLOAT3 specular;
    float shininess;
};
