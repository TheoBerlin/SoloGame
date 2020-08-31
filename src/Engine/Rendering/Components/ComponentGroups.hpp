#pragma once

#include <Engine/ECS/EntitySubscriber.hpp>
#include <Engine/Physics/Velocity.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Transform.hpp>

class CameraComponents : public IComponentGroup {
public:
    std::vector<ComponentAccess> toVector() const override final {
        return {m_Position, m_Rotation, m_ViewProjectionMatrices, m_Velocity};
    }

public:
    ComponentAccess m_Position                  = {R, g_TIDPosition};
    ComponentAccess m_Rotation                  = {R, g_TIDRotation};
    ComponentAccess m_ViewProjectionMatrices    = {R, g_TIDViewProjectionMatrices};
    ComponentAccess m_Velocity                  = {R, g_TIDVelocity};
};

class PointLightComponents : public IComponentGroup {
public:
    std::vector<ComponentAccess> toVector() const override final {
        return {m_Position, m_PointLight};
    }

public:
    ComponentAccess m_Position      = {R, g_TIDPosition};
    ComponentAccess m_PointLight    = {R, g_TIDPointLight};
};
