#include "Game.hpp"

#include <Game/States/Benchmark.hpp>
#include <Game/States/MainMenu.hpp>
#include <Engine/Utils/Debug.hpp>

#include <argh/argh.h>

bool Game::finalize(const argh::parser& flagParser)
{
    State* pStartingState = nullptr;

    if (flagParser[{"-b", "--benchmark"}]) {
        pStartingState = DBG_NEW Benchmark(&m_StateManager, &m_ECS, m_pDevice, m_Window.getInputHandler(), &m_RuntimeStats, &m_Window);
    } else {
        pStartingState = DBG_NEW MainMenu(&m_StateManager, &m_ECS, m_pDevice, m_Window.getInputHandler());
    }

    m_StateManager.enqueueStateTransition(pStartingState, STATE_TRANSITION::PUSH);
    return pStartingState;
}
