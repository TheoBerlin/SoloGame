#include "PointLight.hpp"

LightHandler::LightHandler(ECSCore* pECS)
    :ComponentHandler(pECS, TID(LightHandler))
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {tid_pointLight, &pointLights}
    };

    this->registerHandler(handlerReg);
}

LightHandler::~LightHandler()
{}

bool LightHandler::initHandler()
{
    return true;
}

void LightHandler::createPointLight(Entity entity, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 light, float radius)
{
    PointLight pointLight;
    pointLight.position = position;
    pointLight.light = light;
    pointLight.radiusReciprocal = 1.0f/radius;
    pointLights.push_back(pointLight, entity);
    this->registerComponent(entity, tid_pointLight);
}
