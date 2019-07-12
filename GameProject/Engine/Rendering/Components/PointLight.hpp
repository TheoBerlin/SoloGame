#pragma once

#include <DirectXMath.h>

struct PointLight
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 light;
    float radius;
};

