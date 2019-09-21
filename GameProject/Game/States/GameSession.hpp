#pragma once

#include <Engine/GameState/State.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/LightSpinner.hpp>
#include <Game/Racer/Components/TrackPosition.hpp>
#include <Game/Racer/Systems/RacerMover.hpp>

class MainMenu;

class GameSession : public State
{
public:
    GameSession(MainMenu* mainMenu);
    ~GameSession();

    void resume();
    void pause();

    void update(float dt);

private:
    Entity camera, renderableObject;

    // Component handlers
    TrackPositionHandler trackPositionHandler;
    TubeHandler tubeHandler;

    // Systems
    LightSpinner lightSpinner;
    RacerMover racerMover;
};
