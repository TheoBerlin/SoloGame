#include "Game.hpp"

#include <Game/States/MainMenu.hpp>

Game::Game(HINSTANCE hInstance)
    :IGame(hInstance),
    mainMenu(new MainMenu(&stateManager, &ecs, display.getDevice()))
{
    stateManager.pushState(mainMenu);

    uiHandler.createPanel(ecs.entityIDGen.genID(), {0.0f, 0.45f}, {0.2f, 0.1f}, {1.0f, 0.3f, 0.3f, 1.0f}, 0.0f);
}

Game::~Game()
{}
