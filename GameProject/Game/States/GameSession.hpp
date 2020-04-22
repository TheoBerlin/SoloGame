#pragma once

#include <Engine/GameState/State.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/LightSpinner.hpp>
#include <Game/Racer/Components/TrackPosition.hpp>
#include <Game/Racer/Systems/RacerMover.hpp>

class InputHandler;
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
    Entity m_Camera, m_RenderableCube;

    // Component handlers
    TrackPositionHandler m_TrackPositionHandler;
    TubeHandler m_TubeHandler;

    // Systems
    LightSpinner m_LightSpinner;
    RacerMover m_RacerMover;

    InputHandler* m_pInputHandler;
};
