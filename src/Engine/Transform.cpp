#include "Transform.hpp"

#include <Engine/Utils/Logger.hpp>

#include <cmath>

TransformHandler::TransformHandler(ECSCore* pECS)
    :ComponentHandler(pECS, TID(TransformHandler))
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {g_TIDPosition,     &m_Positions},
        {g_TIDScale,        &m_Scales},
        {g_TIDRotation,     &m_Rotations},
        {g_TIDWorldMatrix,  &m_WorldMatrices}
    };

    this->registerHandler(handlerReg);
}

TransformHandler::~TransformHandler()
{}

bool TransformHandler::initHandler()
{
    return true;
}

void TransformHandler::createPosition(Entity entity, const DirectX::XMFLOAT3& position)
{
    m_Positions.push_back({position}, entity);
    this->registerComponent(entity, g_TIDPosition);
}

void TransformHandler::createScale(Entity entity, const DirectX::XMFLOAT3& scale)
{
    m_Scales.push_back({scale}, entity);
    this->registerComponent(entity, g_TIDScale);
}

void TransformHandler::createRotation(Entity entity)
{
    Rotation rotation = {};
    DirectX::XMStoreFloat4(&rotation.Quaternion, DirectX::XMQuaternionIdentity());
    m_Rotations.push_back(rotation, entity);
    this->registerComponent(entity, g_TIDRotation);
}

void TransformHandler::createTransform(Entity entity, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& scale)
{
    createPosition(entity, position);
    createScale(entity, scale);
    createRotation(entity);
}

void TransformHandler::createWorldMatrix(Entity entity)
{
    if (!m_Positions.hasElement(entity) || !m_Scales.hasElement(entity) || !m_Rotations.hasElement(entity)) {
        LOG_WARNINGF("Tried to create a world matrix component for an entity which does not have a transform: %d", entity);
        return;
    }

    const Transform& transform = getTransform(entity);

    WorldMatrix worldMatrix;
    DirectX::XMStoreFloat4x4(&worldMatrix.worldMatrix,
        DirectX::XMMatrixScaling(transform.Scale.x, transform.Scale.y, transform.Scale.z) *
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&transform.RotationQuaternion)) *
        DirectX::XMMatrixTranslation(transform.Position.x, transform.Position.y, transform.Position.z));

    worldMatrix.dirty = false;

    m_WorldMatrices.push_back(worldMatrix, entity);
    this->registerComponent(entity, g_TIDWorldMatrix);
}

WorldMatrix& TransformHandler::getWorldMatrix(Entity entity)
{
    WorldMatrix& worldMatrix = m_WorldMatrices.indexID(entity);

    if (worldMatrix.dirty) {
        const Transform transform = getTransform(entity);

        DirectX::XMStoreFloat4x4(&worldMatrix.worldMatrix,
            DirectX::XMMatrixScaling(transform.Scale.x, transform.Scale.y, transform.Scale.z) *
            DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&transform.RotationQuaternion)) *
            DirectX::XMMatrixTranslation(transform.Position.x, transform.Position.y, transform.Position.z));

        worldMatrix.dirty = false;
    }

    return worldMatrix;
}

DirectX::XMVECTOR TransformHandler::getUp(const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&rotationQuat);
    return DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(g_DefaultUp, quat));
}

DirectX::XMVECTOR TransformHandler::getForward(const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&rotationQuat);
    return DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(g_DefaultForward, quat));
}

DirectX::XMFLOAT4 TransformHandler::getRotationQuaternion(const DirectX::XMFLOAT3& forward)
{
    DirectX::XMVECTOR temp = DirectX::XMLoadFloat3(&forward);
    DirectX::XMFLOAT4 rotationQuaternion;

    // Rotation angle
    DirectX::XMVECTOR rotationAngleV = DirectX::XMVector3AngleBetweenNormals(g_DefaultForward, temp);
    float rotationAngle = DirectX::XMVectorGetX(temp);

    if (rotationAngle < 0.0f + FLT_EPSILON * 10.0f) {
        temp = DirectX::XMQuaternionIdentity();
    } else {
        // Rotation axis
        temp = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(g_DefaultForward, temp));

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
    DirectX::XMVECTOR temp = DirectX::XMVector3Normalize({g_DefaultForward.m128_f32[0], forward.m128_f32[1], g_DefaultForward.m128_f32[2], 0.0f});
    temp = DirectX::XMVector3AngleBetweenNormals(forward, temp);

    float pitch = DirectX::XMVectorGetX(temp);
    return forward.m128_f32[1] > 0.0f ? -pitch : pitch;
}

float TransformHandler::getRoll(const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR forward = getForward(rotationQuat);
    DirectX::XMVECTOR pitchAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(g_DefaultUp, forward));

    DirectX::XMVECTOR up = getUp(rotationQuat);
    // Up vector without any roll
    DirectX::XMVECTOR upWoRoll = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forward, pitchAxis));

    float roll = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(up, upWoRoll));
    int rollSign = DirectX::XMVectorGetX(DirectX::XMVector3Dot(up, pitchAxis)) < 0.0f;

    // Invert the roll angle if negative
    return roll - (rollSign * 2.0f * roll);
}

float TransformHandler::getOrientedAngle(const DirectX::XMVECTOR& V1, const DirectX::XMVECTOR& V2, const DirectX::XMVECTOR& normal)
{
    float angle = std::acosf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(V1, V2)));
    DirectX::XMVECTOR cross = DirectX::XMVector3Cross(V1, V2);
    if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(cross, normal)) > 0.0f) {
        angle = -angle;
    }

    return angle;
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
        rotation = DirectX::XMQuaternionRotationNormal(g_DefaultUp, DirectX::XM_PI);
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

Transform TransformHandler::getTransform(Entity entity)
{
    return {
        m_Positions.indexID(entity).Position,
        m_Scales.indexID(entity).Scale,
        m_Rotations.indexID(entity).Quaternion
    };
}
