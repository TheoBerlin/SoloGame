#include "Transform.hpp"

TransformHandler::TransformHandler(SystemSubscriber* sysSubscriber)
    :ComponentHandler({tid_transform, tid_worldMatrix}, sysSubscriber, std::type_index(typeid(TransformHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_transform, [this](Entity entity) {return transforms.hasElement(entity);}, &transforms.getIDs()},
        {tid_worldMatrix, [this](Entity entity) {return worldMatrices.hasElement(entity);}, &worldMatrices.getIDs()}
    };

    this->registerHandler(&compRegs);
}

TransformHandler::~TransformHandler()
{}

void TransformHandler::createTransform(Entity entity)
{
    Transform transform;
    transform.position = {0.0f, 0.0f, 0.0f};
    transform.scale = {1.0f, 1.0f, 1.0f};
    transform.rotQuat = {1.0f, 0.0f, 0.0f, 0.0f};

    transforms.push_back(transform, entity);
}

WorldMatrix& TransformHandler::getWorldMatrix(Entity entity)
{
    WorldMatrix& worldMatrix = worldMatrices.indexID(entity);

    if (worldMatrix.dirty) {
        Transform& transform = transforms.indexID(entity);

        DirectX::XMStoreFloat4x4(&worldMatrix.worldMatrix,
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&transform.rotQuat)) *
        DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
        DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z));

        worldMatrix.dirty = false;
    }

    return worldMatrix;
}
