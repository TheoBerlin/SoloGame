#include "State.hpp"

#include <Engine/ECS/ECSCore.hpp>

State::State(State* pOther)
{
    m_pStateManager = pOther->m_pStateManager;
}
