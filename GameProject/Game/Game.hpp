#pragma once

#include <Engine/IGame.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/LightSpinner.hpp>
#include <Game/Racer/Components/Track.hpp>
#include <Game/Racer/Systems/RacerController.hpp>

class MainMenu;

class Game : public IGame
{
public:
    Game();
    ~Game();

    bool finalize() override final;

private:
    MainMenu* m_pMainMenu;
};
