#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/InputHandler.hpp>
#include <DirectXMath.h>

class TrackPositionHandler;
class TransformHandler;
class TubeHandler;

const float racerSpeed = 3.0f;
// Radians per second
const float rotationSpeed = DirectX::XM_PIDIV4;

class RacerMover : public System
{
public:
    RacerMover(ECSInterface* ecs);
    ~RacerMover();

    void update(float dt);

private:
    void onRacerAdded(Entity entity);

    IDVector<Entity> racers;

    TransformHandler* transformHandler;
    TrackPositionHandler* trackPositionHandler;
    TubeHandler* tubeHandler;

    DirectX::Keyboard::State* keyboardState;
};
