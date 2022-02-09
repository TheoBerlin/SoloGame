#pragma once

#include <Engine/ECS/Component.hpp>
#include <DirectXMath.h>

struct PointLightComponent {
    DECL_COMPONENT(PointLightComponent);
    float RadiusReciprocal;
    DirectX::XMFLOAT3 Light;
};
