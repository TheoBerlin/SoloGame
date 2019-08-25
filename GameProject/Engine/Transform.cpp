#include "Transform.hpp"

#include <Engine/Utils/Logger.hpp>

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
    DirectX::XMStoreFloat4(&transform.rotQuat, DirectX::XMQuaternionIdentity());

    transforms.push_back(transform, entity);
    this->registerComponent(tid_transform, entity);
}

void TransformHandler::createWorldMatrix(Entity entity)
{
    if (!transforms.hasElement(entity)) {
        Logger::LOG_WARNING("Tried to create a world matrix component for an entity which does not have a transform: %d", entity);
        return;
    }

    Transform& transform = transforms.indexID(entity);

    WorldMatrix worldMatrix;
    DirectX::XMStoreFloat4x4(&worldMatrix.worldMatrix,
        DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&transform.rotQuat)) *
        DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z));

    worldMatrix.dirty = false;

    worldMatrices.push_back(worldMatrix, entity);
    this->registerComponent(tid_worldMatrix, entity);
}

WorldMatrix& TransformHandler::getWorldMatrix(Entity entity)
{
    WorldMatrix& worldMatrix = worldMatrices.indexID(entity);

    if (worldMatrix.dirty) {
        Transform& transform = transforms.indexID(entity);

        DirectX::XMStoreFloat4x4(&worldMatrix.worldMatrix,
            DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
            DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&transform.rotQuat)) *
            DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z));

        worldMatrix.dirty = false;
    }

    return worldMatrix;
}

DirectX::XMVECTOR TransformHandler::getUp(DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&rotationQuat);
    return DirectX::XMVector3Rotate(defaultUp, quat);
}

DirectX::XMVECTOR TransformHandler::getForward(DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&rotationQuat);
    return DirectX::XMVector3Rotate(defaultForward, quat);
}

DirectX::XMFLOAT4 TransformHandler::getRotationQuaternion(DirectX::XMFLOAT3& forward)
{
    DirectX::XMVECTOR temp = DirectX::XMLoadFloat3(&forward);
    DirectX::XMFLOAT4 rotationQuaternion;

    // Rotation angle
    DirectX::XMVECTOR rotationAngleV = DirectX::XMVector3AngleBetweenNormals(defaultForward, temp);
    float rotationAngle = DirectX::XMVectorGetX(temp);

    if (rotationAngle < 0.0f + FLT_EPSILON * 10.0f) {
        temp = DirectX::XMQuaternionIdentity();
    } else {
        // Rotation axis
        temp = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(defaultForward, temp));

        temp = DirectX::XMQuaternionRotationNormal(temp, rotationAngle);
    }

    DirectX::XMStoreFloat4(&rotationQuaternion, temp);
    return rotationQuaternion;
}

float TransformHandler::getPitch(DirectX::XMVECTOR& forward) const
{
    DirectX::XMVECTOR temp = DirectX::XMVector3Normalize({forward.m128_f32[0], 0.0f, forward.m128_f32[2], 0.0f});
    temp = DirectX::XMVector3AngleBetweenNormals(forward, temp);

    float pitch = DirectX::XMVectorGetX(temp);
    return forward.m128_f32[1] > 0.0f ? -pitch : pitch;
}

void TransformHandler::rotateAroundPoint(const DirectX::XMVECTOR& P, DirectX::XMVECTOR& V, const DirectX::XMVECTOR& axis, float angle)
{
    V = DirectX::XMVectorSubtract(V, P);

    V = DirectX::XMVector3Rotate(V, DirectX::XMQuaternionRotationAxis(axis, angle));

    V = DirectX::XMVectorAdd(V, P);
}
