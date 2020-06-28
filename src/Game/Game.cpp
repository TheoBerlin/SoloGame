#include "Game.hpp"

#include <Game/States/MainMenu.hpp>
#include <Engine/Utils/Debug.hpp>

Game::Game()
    :IGame(),
    m_pMainMenu(nullptr)
{}

Game::~Game()
{}

bool Game::finalize()
{
    m_pMainMenu = DBG_NEW MainMenu(&m_StateManager, &m_ECS, m_pDevice, m_Window.getInputHandler());
    return m_pMainMenu;
}
