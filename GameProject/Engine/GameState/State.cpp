#include "State.hpp"

#include <Engine/ECS/ECSCore.hpp>

State::State(StateManager* pStateManager, ECSCore* pECS, STATE_TRANSITION transitionSetting)
    :m_pECS(pECS),
    m_pStateManager(pStateManager)
{
    m_pStateManager->transitionState(this, transitionSetting);
}

State::State(State* pOther, STATE_TRANSITION transitionSetting)
    :m_pECS(pOther->m_pECS),
    m_pStateManager(pOther->m_pStateManager)
{
    m_pStateManager->transitionState(this, transitionSetting);
}

State::~State()
{}
