#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

struct PointLight
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 light;
    float radius;
};

const std::type_index tid_pointLight = std::type_index(typeid(PointLight));

class LightHandler : public ComponentHandler
{
public:
    LightHandler(SystemSubscriber* sysSubscriber);
    ~LightHandler();

    void createPointLight(Entity entity, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 light, float radius);

    IDVector<PointLight> pointLights;
};
