#pragma once

#include <Engine/ECS/System.hpp>

#include <DirectXMath.h>

class InputHandler;
class TubeHandler;

constexpr const float g_RacerMinSpeed = 1.0f;
constexpr const float g_RacerMaxSpeed = 4.5f;

constexpr const float g_RacerAcceleration = 1.5f;

// Radians per second
constexpr const float g_RotationSpeed = DirectX::XM_PIDIV2 * 0.8f;

// Racers' minimum distance to the tube's edge or center
constexpr const float g_MinEdgeDistance = 0.3f;
constexpr const float g_MinCenterDistance = 0.05f;

// Speed at which a racer moves towards or away from the center
constexpr const float g_CenterMoveSpeed = 1.0f;

// Racers' starting distance from the tube's center, [0, 1]
constexpr const float g_StartingCenterDistance = 0.5f;

class RacerController : System
{
public:
    RacerController(TubeHandler* pTubeHandler);
    ~RacerController() = default;

    void Update(float dt) override final;

private:
    void OnRacerAdded(Entity entity);

private:
    IDVector m_Racers;

    TubeHandler* m_pTubeHandler;
};
