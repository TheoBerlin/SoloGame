#pragma once

#include <Engine/GameState/StateManager.hpp>

class ECSCore;

class State
{
public:
    State(StateManager* pStateManager, ECSCore* pECS, STATE_TRANSITION transitionSetting);
    State(State* pOther, STATE_TRANSITION transitionSetting);
    virtual ~State();

    virtual void resume() = 0;
    virtual void pause() = 0;

    virtual void update(float dt) = 0;

protected:
    ECSCore* m_pECS;
    StateManager* m_pStateManager;
};
