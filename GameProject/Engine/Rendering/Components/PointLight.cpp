#include "PointLight.hpp"

LightHandler::LightHandler(ECSCore* pECS)
    :ComponentHandler({tid_pointLight}, pECS, std::type_index(typeid(LightHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_pointLight, &pointLights}
    };

    this->registerHandler(&compRegs);
}

LightHandler::~LightHandler()
{}

void LightHandler::createPointLight(Entity entity, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 light, float radius)
{
    PointLight pointLight;
    pointLight.position = position;
    pointLight.light = light;
    pointLight.radiusReciprocal = 1.0f/radius;
    pointLights.push_back(pointLight, entity);
    this->registerComponent(entity, tid_pointLight);
}
