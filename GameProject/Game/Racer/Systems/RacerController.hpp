#pragma once

#include <Engine/ECS/System.hpp>

#include <DirectXMath.h>

class InputHandler;
class TrackHandler;
class TransformHandler;
class TubeHandler;
class VelocityHandler;

const float g_RacerMinSpeed = 1.0f;
const float g_RacerMaxSpeed = 4.5f;

const float g_RacerAcceleration = 1.5f;

// Radians per second
const float rotationSpeed = DirectX::XM_PIDIV2 * 0.8f;

// Racers' minimum distance to the tube's edge or center
const float minEdgeDistance = 0.3f;
const float minCenterDistance = 0.05f;

// Speed at which a racer moves towards or away from the center
const float centerMoveSpeed = 1.0f;

// Racers' starting distance from the tube's center, [0, 1]
const float startingCenterDistance = 0.5f;

class RacerController : public System
{
public:
    RacerController(ECSCore* pECS, InputHandler* pInputHandler, TubeHandler* pTubeHandler);
    ~RacerController();

    virtual bool initSystem() override;

    void update(float dt);

private:
    void racerAdded(Entity entity);

private:
    IDVector m_Racers;

    InputHandler* m_pInputHandler;
    TransformHandler* m_pTransformHandler;
    TrackHandler* m_pTrackHandler;
    TubeHandler* m_pTubeHandler;
    VelocityHandler* m_pVelocityHandler;
};
