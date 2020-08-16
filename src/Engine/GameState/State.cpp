#include "State.hpp"

#include <Engine/ECS/ECSCore.hpp>

State::State(StateManager* pStateManager, ECSCore* pECS)
    :m_pECS(pECS),
    m_pStateManager(pStateManager)
{}

State::State(State* pOther)
    :m_pECS(pOther->m_pECS),
    m_pStateManager(pOther->m_pStateManager)
{}
