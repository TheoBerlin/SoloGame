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
    m_pMainMenu = DBG_NEW MainMenu(&m_StateManager, &m_ECS, &m_Device, m_Window.getInputHandler());

    m_pUICore->getPanelHandler().createPanel(m_ECS.createEntity(), {0.0f, 0.45f}, {0.2f, 0.1f}, {1.0f, 0.3f, 0.3f, 1.0f}, 0.0f);

    return m_pMainMenu;
}
