#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

struct PointLight {
    float RadiusReciprocal;
    DirectX::XMFLOAT3 Light;
};

const std::type_index g_TIDPointLight = TID(PointLight);

class LightHandler : public ComponentHandler
{
public:
    LightHandler(ECSCore* pECS);
    ~LightHandler();

    virtual bool initHandler() override;

    inline PointLight& getPointLight(Entity entity) { return m_PointLights.indexID(entity); }

    void createPointLight(Entity entity, const DirectX::XMFLOAT3& light, float radius);

private:
    IDDVector<PointLight> m_PointLights;
};
