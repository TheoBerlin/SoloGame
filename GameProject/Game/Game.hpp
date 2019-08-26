#pragma once

#include <Engine/IGame.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/LightSpinner.hpp>
#include <Game/Racer/Components/TrackPosition.hpp>
#include <Game/Racer/Systems/RacerMover.hpp>

class Game : public IGame
{
public:
    Game(HINSTANCE hInstance);
    ~Game();

    void update(float dt);

private:
    bool hasSetup;

    Entity camera, renderableObject;

    LightSpinner lightSpinner;
    TubeHandler tubeHandler;
    TrackPositionHandler trackPositionHandler;
    RacerMover racerMover;
};
