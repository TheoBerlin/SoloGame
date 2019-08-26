#pragma once

#include <Engine/ECS/System.hpp>

class TrackPositionHandler;
class TransformHandler;
class TubeHandler;

const float racerSpeed = 5.0f;

class RacerMover : public System
{
public:
    RacerMover(ECSInterface* ecs);
    ~RacerMover();

    void update(float dt);

private:
    IDVector<Entity> racers;

    TransformHandler* transformHandler;
    TrackPositionHandler* trackPositionHandler;
    TubeHandler* tubeHandler;
};
