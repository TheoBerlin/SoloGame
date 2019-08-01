#include "PointLight.hpp"

LightHandler::LightHandler(SystemSubscriber* sysSubscriber)
    :ComponentHandler({tid_pointLight}, sysSubscriber, std::type_index(typeid(LightHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_pointLight, [this](Entity entity) {return pointLights.hasElement(entity);}, &pointLights.getIDs()}
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
    pointLight.radius = radius;
    this->registerComponent(tid_pointLight, entity);
}
