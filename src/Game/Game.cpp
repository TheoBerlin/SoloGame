#include "Game.hpp"

#include "Game/States/BenchmarkState.hpp"
#include "Game/States/MainMenuState.hpp"

#include <argh/argh.h>

bool Game::Finalize(const argh::parser& flagParser)
{
    State* pStartingState = nullptr;

    if (flagParser[{"-b", "--benchmark"}]) {
        pStartingState = DBG_NEW BenchmarkState(&m_StateManager, &m_RuntimeStats);
    } else {
        pStartingState = DBG_NEW MainMenuState(&m_StateManager);
    }

    m_StateManager.EnqueueStateTransition(pStartingState, STATE_TRANSITION::PUSH);
    return pStartingState;
}
