#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/InputHandler.hpp>
#include <DirectXMath.h>

class TrackPositionHandler;
class TransformHandler;
class TubeHandler;

const float racerSpeed = 3.0f;
// Radians per second
const float rotationSpeed = DirectX::XM_PIDIV2 * 0.8f;
// Racers' minimum distance to the tube's edge or center
const float minEdgeDistance = 0.3f;
const float minCenterDistance = 0.05f;
// Speed at which a racer moves towards or away from the center
const float centerMoveSpeed = 1.0f;
// Racers' starting distance from the tube's center, where 0 is nearest allowed distance, and 1 is the furthest
const float startingCenterDistance = 0.5f;

class RacerMover : public System
{
public:
    RacerMover(ECSCore* pECS);
    ~RacerMover();

    void update(float dt);

private:
    void racerAdded(Entity entity);

    IDVector<Entity> racers;

    TransformHandler* transformHandler;
    TrackPositionHandler* trackPositionHandler;
    TubeHandler* tubeHandler;

    DirectX::Keyboard::State* keyboardState;
};
