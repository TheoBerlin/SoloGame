#pragma once

#include <Engine/ECS/ComponentSubscriptionRequest.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Transform.hpp>

class CameraComponents : public IComponentGroup {
public:
    std::vector<ComponentAccess> toVector() const override final
    {
        return {m_Position, m_Rotation, m_ViewMatrix, m_ProjectionMatrix};
    }

public:
    ComponentAccess m_Position          = {R, g_TIDPosition};
    ComponentAccess m_Rotation          = {R, g_TIDRotation};
    ComponentAccess m_ViewMatrix        = {R, tid_view};
    ComponentAccess m_ProjectionMatrix  = {R, tid_projection};
};
