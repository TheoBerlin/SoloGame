#pragma once

#include "Engine/GameState/StateManager.hpp"

class StateManager;

class State
{
public:
	State(StateManager* pStateManager) : m_pStateManager(pStateManager) {}
	State(State* pOther);
	virtual ~State() = 0 {};

	virtual void Init() = 0;

	virtual void Resume() = 0;
	virtual void Pause() = 0;

	virtual void Update(float delta) = 0;

protected:
	StateManager* m_pStateManager;
};
