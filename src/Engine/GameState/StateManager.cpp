#include "StateManager.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/GameState/State.hpp>

StateManager::StateManager()
	:m_pEnqueuedState(nullptr),
	m_EnqueuedTransitionAction(STATE_TRANSITION::PUSH)
{}

StateManager::~StateManager()
{
	Release();
}

void StateManager::Release()
{
	while (!m_StatesToDelete.empty()) {
		SAFEDELETE(m_StatesToDelete.front());
		m_StatesToDelete.pop();
	}

	ECSCore* pECS = ECSCore::GetInstance();
	while (!m_States.empty()) {
		SAFEDELETE(m_States.top());
		pECS->DeleteTopRegistryPage();
		m_States.pop();
	}
}

void StateManager::EnqueueStateTransition(State* pNewState, STATE_TRANSITION transitionSetting)
{
	m_pEnqueuedState = pNewState;
	m_EnqueuedTransitionAction = transitionSetting;
}

void StateManager::Update(float dt)
{
	if (!m_States.empty()) {
		m_States.top()->Update(dt);
	}

	if (m_pEnqueuedState) {
		TransitionState();
	}
}

void StateManager::TransitionState()
{
	ECSCore* pECS = ECSCore::GetInstance();

	switch (m_EnqueuedTransitionAction) {
		case STATE_TRANSITION::PUSH:
			pECS->AddRegistryPage();
			m_States.push(m_pEnqueuedState);
			break;
		case STATE_TRANSITION::POP:
			SAFEDELETE(m_States.top());
			pECS->DeleteTopRegistryPage();
			m_States.pop();

			if (!m_States.empty()) {
				pECS->ReinstateTopRegistryPage();
				m_States.top()->Resume();
			}
			break;
		case STATE_TRANSITION::PAUSE_AND_PUSH:
			m_States.top()->Pause();
			pECS->DeregisterTopRegistryPage();

			pECS->AddRegistryPage();
			m_States.push(m_pEnqueuedState);
			break;
		case STATE_TRANSITION::POP_AND_PUSH:
			pECS->DeleteTopRegistryPage();
			delete m_States.top();
			m_States.pop();

			pECS->AddRegistryPage();
			m_States.push(m_pEnqueuedState);
			break;
	}

	m_pEnqueuedState->Init();
	m_pEnqueuedState = nullptr;
}
