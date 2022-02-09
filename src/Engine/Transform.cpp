#include "Transform.hpp"

#include <cmath>

DirectX::XMFLOAT4X4 CreateWorldMatrix(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMStoreFloat4x4(&worldMatrix,
        DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
        DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotationQuat)) *
        DirectX::XMMatrixTranslation(position.x, position.y, position.z));

    return worldMatrix;
}

DirectX::XMVECTOR GetUp(const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&rotationQuat);
    return DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(g_DefaultUp, quat));
}

DirectX::XMVECTOR GetForward(const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&rotationQuat);
    return DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(g_DefaultForward, quat));
}

DirectX::XMFLOAT4 GetRotationQuaternion(const DirectX::XMFLOAT3& forward)
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

float GetPitch(const DirectX::XMVECTOR& forward)
{
    DirectX::XMVECTOR temp = DirectX::XMVector3Normalize({forward.m128_f32[0], 0.0f, forward.m128_f32[2], 0.0f});
    temp = DirectX::XMVector3AngleBetweenNormals(forward, temp);

    float pitch = DirectX::XMVectorGetX(temp);
    return forward.m128_f32[1] > 0.0f ? -pitch : pitch;
}

float GetYaw(const DirectX::XMVECTOR& forward)
{
    DirectX::XMVECTOR temp = DirectX::XMVector3Normalize({g_DefaultForward.m128_f32[0], forward.m128_f32[1], g_DefaultForward.m128_f32[2], 0.0f});
    temp = DirectX::XMVector3AngleBetweenNormals(forward, temp);

    float pitch = DirectX::XMVectorGetX(temp);
    return forward.m128_f32[1] > 0.0f ? -pitch : pitch;
}

float GetRoll(const DirectX::XMFLOAT4& rotationQuat)
{
    DirectX::XMVECTOR forward = GetForward(rotationQuat);
    DirectX::XMVECTOR pitchAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(g_DefaultUp, forward));

    DirectX::XMVECTOR up = GetUp(rotationQuat);
    // Up vector without any roll
    DirectX::XMVECTOR upWoRoll = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forward, pitchAxis));

    float roll = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(up, upWoRoll));
    int rollSign = DirectX::XMVectorGetX(DirectX::XMVector3Dot(up, pitchAxis)) < 0.0f;

    // Invert the roll angle if negative
    return roll - (rollSign * 2.0f * roll);
}

float GetOrientedAngle(const DirectX::XMVECTOR& V1, const DirectX::XMVECTOR& V2, const DirectX::XMVECTOR& normal)
{
    float angle = std::acosf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(V1, V2)));
    DirectX::XMVECTOR cross = DirectX::XMVector3Cross(V1, V2);
    if (DirectX::XMVectorGetX(DirectX::XMVector3Dot(cross, normal)) > 0.0f) {
        angle = -angle;
    }

    return angle;
}

void SetForward(DirectX::XMFLOAT4& rotationQuat, const DirectX::XMVECTOR& forward)
{
	// Create rotation quaternion based on new forward
	// Beware of the cases where the new forward vector is parallell to the old one
    DirectX::XMVECTOR currentForward = GetForward(rotationQuat);
	float cosAngle = DirectX::XMVectorGetX(DirectX::XMVector3Dot(forward, currentForward));

    DirectX::XMVECTOR rotation;

	if (cosAngle >= 1.0f - FLT_EPSILON * 10.0f) {
		// The new forward is identical to the old one, do nothing
		return;
	} else if (cosAngle <= -1.0f + FLT_EPSILON * 10.0f) {
		// The new forward is parallell to the old one, create a 180 degree rotation quarternion
		// around any axis
        rotation = DirectX::XMQuaternionRotationNormal(g_DefaultUp, DirectX::XM_PI);
	} else {
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

void Roll(DirectX::XMFLOAT4& rotationQuat, float angle)
{
    DirectX::XMVECTOR forward = GetForward(rotationQuat);
    DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationAxis(forward, angle);

    DirectX::XMVECTOR currentRotQuat = DirectX::XMLoadFloat4(&rotationQuat);
    DirectX::XMStoreFloat4(&rotationQuat, DirectX::XMQuaternionMultiply(currentRotQuat, rotation));
}

void RotateAroundPoint(const DirectX::XMVECTOR& P, DirectX::XMVECTOR& V, const DirectX::XMVECTOR& axis, float angle)
{
    V = DirectX::XMVectorSubtract(V, P);

    V = DirectX::XMVector3Rotate(V, DirectX::XMQuaternionRotationAxis(axis, angle));

    V = DirectX::XMVectorAdd(V, P);
}
