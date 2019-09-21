#include "Game.hpp"

#include <Game/States/MainMenu.hpp>

Game::Game(HINSTANCE hInstance)
    :IGame(hInstance),
    mainMenu(new MainMenu(&stateManager, &ecs, display.getDevice()))
{
    stateManager.pushState(mainMenu);
}

Game::~Game()
{}
