#include "StateManager.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/GameState/State.hpp>

StateManager::StateManager(ECSCore* pECS)
    :m_pECS(pECS),
    m_pEnqueuedState(nullptr),
    m_EnqueuedTransitionAction(STATE_TRANSITION::PUSH)
{}

StateManager::~StateManager()
{
    while (!m_StatesToDelete.empty()) {
        delete m_StatesToDelete.front();
        m_StatesToDelete.pop();
    }

    while (!m_States.empty()) {
        delete m_States.top();
        m_States.pop();
    }
}

void StateManager::enqueueStateTransition(State* pNewState, STATE_TRANSITION transitionSetting)
{
    m_pEnqueuedState = pNewState;
    m_EnqueuedTransitionAction = transitionSetting;
}

void StateManager::update(float dt)
{
    if (!m_States.empty()) {
        m_States.top()->update(dt);
    }

    if (m_pEnqueuedState) {
        transitionState();
    }
}

void StateManager::transitionState()
{
    switch (m_EnqueuedTransitionAction) {
        case STATE_TRANSITION::PUSH:
            m_pECS->addRegistryPage();
            m_States.push(m_pEnqueuedState);
            break;
        case STATE_TRANSITION::POP:
            delete m_States.top();
            m_pECS->deleteTopRegistryPage();
            m_States.pop();

            if (!m_States.empty()) {
                m_pECS->reinstateTopRegistryPage();
                m_States.top()->resume();
            }
            break;
        case STATE_TRANSITION::PAUSE_AND_PUSH:
            m_States.top()->pause();
            m_pECS->deregisterTopRegistryPage();

            m_pECS->addRegistryPage();
            m_States.push(m_pEnqueuedState);
            break;
        case STATE_TRANSITION::POP_AND_PUSH:
            m_pECS->deleteTopRegistryPage();
            m_StatesToDelete.push(m_States.top());
            m_States.pop();

            m_pECS->addRegistryPage();
            m_States.push(m_pEnqueuedState);
            break;
    }

    m_pEnqueuedState->init();
    m_pEnqueuedState = nullptr;
}
