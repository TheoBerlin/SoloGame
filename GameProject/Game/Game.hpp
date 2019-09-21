#pragma once

#include <Engine/IGame.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/LightSpinner.hpp>
#include <Game/Racer/Components/TrackPosition.hpp>
#include <Game/Racer/Systems/RacerMover.hpp>

class MainMenu;

class Game : public IGame
{
public:
    Game(HINSTANCE hInstance);
    ~Game();

private:
    MainMenu* mainMenu;
};
