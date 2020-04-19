#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

struct PointLight
{
    DirectX::XMFLOAT3 position;
    float radiusReciprocal;
    DirectX::XMFLOAT3 light;
    float padding;
};

const std::type_index tid_pointLight = TID(PointLight);

class LightHandler : public ComponentHandler
{
public:
    LightHandler(ECSCore* pECS);
    ~LightHandler();

    virtual bool init() override;

    void createPointLight(Entity entity, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 light, float radius);

    IDDVector<PointLight> pointLights;
};
