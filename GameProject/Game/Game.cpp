#include "Game.hpp"

#include <Game/States/MainMenu.hpp>

Game::Game(HINSTANCE hInstance)
    :IGame(hInstance),
    mainMenu(new MainMenu(&m_StateManager, &m_ECS, m_Display.getDevice()))
{
    m_UIHandler.createPanel(m_ECS.createEntity(), {0.0f, 0.45f}, {0.2f, 0.1f}, {1.0f, 0.3f, 0.3f, 1.0f}, 0.0f);
}

Game::~Game()
{}
