#include "Transform.hpp"

#include <Engine/Utils/Logger.hpp>

#include <cmath>

TransformHandler::TransformHandler(ECSCore* pECS)
    :ComponentHandler({tid_transform, tid_worldMatrix}, pECS, TID(TransformHandler))
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {tid_transform, &transforms},
        {tid_worldMatrix, &worldMatrices}
    };

    this->registerHandler(handlerReg);
}

TransformHandler::~TransformHandler()
{}

bool TransformHandler::init()
{
    return true;
}

void TransformHandler::createTransform(Entity entity)
{
    Transform transform;
    transform.position = {0.0f, 0.0f, 0.0f};
    transform.scale = {1.0f, 1.0f, 1.0f};
    DirectX::XMStoreFloat4(&transform.rotQuat, DirectX::XMQuaternionIdentity());

    transforms.push_back(transform, entity);
    this->registerComponent(entity, tid_transform);
}

void TransformHandler::createWorldMatrix(Entity entity)
{
    if (!transforms.hasElement(entity)) {
        Log_Warning("Tried to create a world matrix component for an entity which does not have a transform: %d", entity);
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
    this->registerComponent(entity, tid_worldMatrix);
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

DirectX::XMVECTOR TransformHandler::getUp(const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&rotationQuat);
    return DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(defaultUp, quat));
}

DirectX::XMVECTOR TransformHandler::getForward(const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&rotationQuat);
    return DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(defaultForward, quat));
}

DirectX::XMFLOAT4 TransformHandler::getRotationQuaternion(const DirectX::XMFLOAT3& forward)
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

float TransformHandler::getPitch(const DirectX::XMVECTOR& forward)
{
    DirectX::XMVECTOR temp = DirectX::XMVector3Normalize({forward.m128_f32[0], 0.0f, forward.m128_f32[2], 0.0f});
    temp = DirectX::XMVector3AngleBetweenNormals(forward, temp);

    float pitch = DirectX::XMVectorGetX(temp);
    return forward.m128_f32[1] > 0.0f ? -pitch : pitch;
}

float TransformHandler::getYaw(const DirectX::XMVECTOR& forward)
{
    DirectX::XMVECTOR temp = DirectX::XMVector3Normalize({defaultForward.m128_f32[0], forward.m128_f32[1], defaultForward.m128_f32[2], 0.0f});
    temp = DirectX::XMVector3AngleBetweenNormals(forward, temp);

    float pitch = DirectX::XMVectorGetX(temp);
    return forward.m128_f32[1] > 0.0f ? -pitch : pitch;
}

float TransformHandler::getRoll(const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR forward = getForward(rotationQuat);
    DirectX::XMVECTOR pitchAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(defaultUp, forward));

    DirectX::XMVECTOR up = getUp(rotationQuat);
    // Up vector without any roll
    DirectX::XMVECTOR upWoRoll = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forward, pitchAxis));

    float roll = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(up, upWoRoll));
    int rollSign = DirectX::XMVectorGetX(DirectX::XMVector3Dot(up, pitchAxis)) < 0.0f;

    // Invert the roll angle if negative
    return roll - (rollSign * 2.0f * roll);
}

void TransformHandler::setForward(DirectX::XMFLOAT4& rotationQuat, const DirectX::XMVECTOR& forward)
{
	// Create rotation quaternion based on new forward
	// Beware of the cases where the new forward vector is parallell to the old one
    DirectX::XMVECTOR currentForward = getForward(rotationQuat);
	float cosAngle = DirectX::XMVectorGetX(DirectX::XMVector3Dot(forward, currentForward));

    DirectX::XMVECTOR rotation;

	if (cosAngle >= 1.0f - FLT_EPSILON * 10.0f) {
		// The new forward is identical to the old one, do nothing
		return;
	}
	else if (cosAngle <= -1.0f + FLT_EPSILON * 10.0f) {
		// The new forward is parallell to the old one, create a 180 degree rotation quarternion
		// around any axis
        rotation = DirectX::XMQuaternionRotationNormal(defaultUp, DirectX::XM_PI);
	}
	else {
		// Calculate rotation quaternion
		DirectX::XMVECTOR axis = DirectX::XMVector3Cross(currentForward, forward);

		float axisLength = DirectX::XMVectorGetX(DirectX::XMVector3Length(axis));

        // Is this branching really needed?
		if (axisLength < FLT_EPSILON) {
			return;
		}

		axis = DirectX::XMVectorScale(axis, 1.0f/axisLength);

		float angle = std::acosf(cosAngle);

		rotation = DirectX::XMQuaternionRotationNormal(axis, angle);
	}

    DirectX::XMVECTOR currentRotQuat = DirectX::XMLoadFloat4(&rotationQuat);
    DirectX::XMStoreFloat4(&rotationQuat, DirectX::XMQuaternionMultiply(currentRotQuat, rotation));
}

void TransformHandler::roll(DirectX::XMFLOAT4& rotationQuat, float angle)
{
    DirectX::XMVECTOR forward = getForward(rotationQuat);
    DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationAxis(forward, angle);

    DirectX::XMVECTOR currentRotQuat = DirectX::XMLoadFloat4(&rotationQuat);
    DirectX::XMStoreFloat4(&rotationQuat, DirectX::XMQuaternionMultiply(currentRotQuat, rotation));
}

void TransformHandler::rotateAroundPoint(const DirectX::XMVECTOR& P, DirectX::XMVECTOR& V, const DirectX::XMVECTOR& axis, float angle)
{
    V = DirectX::XMVectorSubtract(V, P);

    V = DirectX::XMVector3Rotate(V, DirectX::XMQuaternionRotationAxis(axis, angle));

    V = DirectX::XMVectorAdd(V, P);
}
